#!/usr/bin/env bash
set -euo pipefail

# scripts/generate_doxygen.sh
# Genera la documentazione Doxygen (HTML + grafi via Graphviz) dai file sorgente
#
# COSA GENERA:
#   - Documentazione HTML interattiva (C/C++ api docs + Markdown docs)
#   - Grafi delle chiamate (Call Graph e Caller Graph per ogni funzione)
#   - Albero gerarchico delle classi (Class Hierarchy Diagrams)
#   - Diagrammi delle dipendenze tra file e directory
#   - Diagrammi e grafi UML con Graphviz
#   - Indici per classi, file, moduli e namespace
#   - File CSS/JS per la navigazione interattiva
#
# OUTPUT:
#   - doxygen/html/          -> Documentazione HTML completa (apribile nel browser)
#   - doxygen/markdown/      -> File Markdown copiati da docs/a_*.md
#
# USO:
#   ./scripts/generate_doxygen.sh [--open]
#     --open : Apre automaticamente la documentazione nel browser

WORKDIR="$(cd "$(dirname "$0")/.." && pwd)"
DOXYFILE="$WORKDIR/Doxyfile"
OUTDIR="$WORKDIR/doxygen"
MD_OUTDIR="$OUTDIR/markdown"
REL_OUTDIR="${OUTDIR#${WORKDIR}/}"
OPEN_BROWSER=false

# Gestione argomenti
if [[ "${1:-}" == "--open" ]]; then
  OPEN_BROWSER=true
fi

command -v doxygen >/dev/null 2>&1 || { echo "doxygen non trovato nel PATH. Installa doxygen e riprova." >&2; exit 1; }
command -v dot >/dev/null 2>&1 || {
  echo "WARNING: graphviz 'dot' non trovato. I grafi non verranno generati." >&2
  echo "Per avere i grafi installa 'graphviz' (es. apt install graphviz)." >&2
}

echo "Working dir: $WORKDIR"
echo "Using Doxyfile: $DOXYFILE"

# Assicura che l'output precedente sia rimosso per avere una rigenerazione pulita
if [ -d "$OUTDIR" ]; then
  echo "Pulisco output precedente: $OUTDIR"
  rm -rf "$OUTDIR"
fi

mkdir -p "$OUTDIR"

# Esegui Doxygen
echo "Eseguo: doxygen $DOXYFILE"
if ! doxygen "$DOXYFILE"; then
    echo "Errore durante l'esecuzione di doxygen" >&2
    exit 1
fi

# Copia i documenti markdown dalla cartella docs
mkdir -p "$MD_OUTDIR"
echo "Copio documentazione markdown da docs/ a $MD_OUTDIR"
echo

# Cerca file markdown da copiare (priorità: a_*.md, poi *.md se espliciti)
FOUND_MD_FILES=0

# Prima prova a copiare i documenti "approvati" che iniziano con a_
if find "$WORKDIR/docs" -maxdepth 3 -name "a_*.md" -type f ! -path "*/Backup/*" -quit &>/dev/null; then
  rsync -av --prune-empty-dirs \
    --exclude='doxygen/***' \
    --exclude='docs/doxygen/***' \
    --exclude='mh1001/***' \
    --exclude='**/node_modules/**' \
    --exclude='docs/Backup/***' \
    --include='docs/' \
    --include='docs/**/' \
    --include='a_*.md' \
    --exclude='docs/***' \
    --exclude='*' \
    "$WORKDIR/" "$MD_OUTDIR/" 2>/dev/null || true
  FOUND_MD_FILES=$(find "$MD_OUTDIR" -type f -name "*.md" 2>/dev/null | wc -l)
fi

# Se non find niente, copia i file markdown standard
if [ "$FOUND_MD_FILES" -eq 0 ]; then
  echo "ℹ️  Nessun documento con naming 'a_*.md' trovato, copio i file markdown standard..."
  for md_file in "$WORKDIR"/docs/*.md; do
    if [ -f "$md_file" ] && [ "$(basename "$md_file")" != "README.md" ]; then
      cp -v "$md_file" "$MD_OUTDIR/" 2>/dev/null || true
    fi
  done
  FOUND_MD_FILES=$(find "$MD_OUTDIR" -type f -name "*.md" 2>/dev/null | wc -l)
fi

if [ "$FOUND_MD_FILES" -eq 0 ]; then
  echo "⚠️  ATTENZIONE: Nessun file markdown trovato da copiare"
  echo "    Crea file markdown nella cartella docs/ con naming:"
  echo "    - Documenti approvati: docs/a_*.md  (consigliato)"
  echo "    - O file standard: docs/*.md"
else
  echo "✓ $FOUND_MD_FILES file markdown copiati"
fi
echo
echo
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║           DOCUMENTAZIONE DOXYGEN GENERATA CON SUCCESSO         ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo

if [ -d "$OUTDIR/html" ]; then
  HTML_COUNT=$(find "$OUTDIR/html" -type f -name "*.html" 2>/dev/null | wc -l)
  SVG_COUNT=$(find "$OUTDIR/html" -type f -name "*.svg" 2>/dev/null | wc -l)
  echo "✓ Documentazione HTML generata: $HTML_COUNT file HTML"
  if [ "$SVG_COUNT" -gt 0 ]; then
    echo "  Grafi SVG generati: $SVG_COUNT"
  fi
  echo "  Percorso: $OUTDIR/html"
  du -sh "$OUTDIR/html" || true
  echo
  echo "📊 Grafi e diagrammi disponibili:"
  echo "  ✓ Call Graph (albero delle chiamate per ogni funzione)"
  echo "  ✓ Caller Graph (chi chiama ogni funzione)"
  echo "  ✓ Class Hierarchy (albero gerarchico delle classi)"
  echo "  ✓ Directory Dependency (dipendenze tra directory)"
  echo "  ✓ Include Dependency (dipendenze tra file)"
  echo
  echo "📄 File HTML principali:"
  echo "  - Index: file://$OUTDIR/html/index.html"
  echo "  - Files: file://$OUTDIR/html/files.html"
  echo "  - Classes: file://$OUTDIR/html/classes.html"
  echo "  - Modules: file://$OUTDIR/html/modules.html"
  echo "  - Dependencies: file://$OUTDIR/html/files.html (seleziona un file per vedere i grafi)"
  echo
  echo "Puoi aprire la documentazione nel browser con:"
  echo "  firefox $OUTDIR/html/index.html"
  echo "  o semplicemente doppio-click su: $OUTDIR/html/index.html"
else
  echo "⚠ ATTENZIONE: Directory HTML non trovata: $OUTDIR/html"
fi

if [ -d "$MD_OUTDIR" ]; then
  MD_COUNT=$(find "$MD_OUTDIR" -type f -name "*.md" 2>/dev/null | wc -l)
  if [ "$MD_COUNT" -gt 0 ]; then
    echo
    echo "✓ File Markdown copiati: $MD_COUNT file"
    echo "  Percorso: $MD_OUTDIR"
    echo "  Contenuti:"
    find "$MD_OUTDIR" -type f -name "*.md" -printf "    - %f\n" 2>/dev/null | sort
  fi
fi

echo
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                    RIEPILOGO GENERAZIONE                        ║"
echo "╚════════════════════════════════════════════════════════════════╝"
if [ -d "$OUTDIR" ]; then
  du -sh "$OUTDIR" || true
  echo
  echo "Top 10 file per dimensione:"
  find "$OUTDIR" -type f -printf '%s %p\n' 2>/dev/null | sort -rn | head -n 10 | awk '{printf "%8.2fMB  %s\n", $1/(1024*1024), $2}'
else
  echo "⚠ Output non trovato: $OUTDIR"
fi

echo
echo "═══════════════════════════════════════════════════════════════════"
echo "Documentazione disponibile in: $OUTDIR"
echo "═══════════════════════════════════════════════════════════════════"
echo

# Suggerimenti per navigare i call graph
echo "💡 SUGGERIMENTI PER LA NAVIGAZIONE:"
echo
echo "1. Call Graph (albero delle chiamate):"
echo "   → Apri il file index.html e cerca una funzione"
echo "   → Scorri fino a trovare la sezione 'Call graph for this function'"
echo "   → Clicca sul grafo per vederlo in dettaglio"
echo
echo "2. Caller Graph (chi chiama questa funzione):"
echo "   → Nella stessa pagina, sotto 'Caller graph for this function'"
echo "   → Mostra tutte le funzioni che chiamano questa funzione"
echo
echo "3. Dipendenze tra file:"
echo "   → Vai su Files → Seleziona un file"
echo "   → Vedi il grafo 'include dependency graph for this file'"
echo
echo "4. Class Hierarchy:"
echo "   → Vai su Classes → Hierarchy"
echo "   → Visualizza l'albero gerarchico di tutte le classi"
echo
echo "5. Directory Dependencies:"
echo "   → Vai su Files → Directories"
echo "   → Vedi le dipendenze tra directory del progetto"
echo

# Apri il browser se richiesto
if [ "$OPEN_BROWSER" = true ] && [ -f "$OUTDIR/html/index.html" ]; then
  echo "Apertura della documentazione nel browser..."
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "file://$OUTDIR/html/index.html" &
  elif command -v open >/dev/null 2>&1; then
    open "file://$OUTDIR/html/index.html" &
  elif command -v firefox >/dev/null 2>&1; then
    firefox "file://$OUTDIR/html/index.html" &
  else
    echo "⚠ Impossibile aprire il browser automaticamente. Apri manualmente:"
    echo "  file://$OUTDIR/html/index.html"
  fi
fi

exit 0
