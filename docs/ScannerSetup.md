Use of Programming Command
Besides the barcode programming method, the engine can also be configured by serial commands (HEX) sent from the
host device. All commands must be entered in uppercase letters.
Query Commands
For query commands, the entry in the Data field in the syntax above is one of the following characters means:
* (HEX: 2A) What is the engine’s current value for the setting(s).
& (HEX: 26) What is the factory default value for the setting(s).
^ (HEX: 5E) What is the range of possible values for the setting(s).

The value of the StoreType field in a query command can be either “@” (HEX: 40) or “#” (HEX: 23).
A query command with the SubTag field omitted means to query all the settings concerning a tag. For example, to query
all the current settings about Code 11, you should enter 7E 01 30 30 30 30 40 43 31 31 2A 3B 03 (i.e.
~<SOH>0000@C11*;<ETX>).
Command Syntax
Prefix StorageType Tag SubTag {Data} [,SubTag {Data}] [;Tag SubTag {Data}] […] Suffix
Prefix: “~<SOH>0000” (HEX: 7E 01 30 30 30 30), 6 characters.
StorageType: “@” (HEX: 40) or “#” (HEX: 23), 1 character. “@” means permanent setting which will not be lost by removing
power from the engine or rebooting it; “#” means temporary setting which will be lost by removing power from the engine or
rebooting it.
Tag: A 3-character case-sensitive field that identifies the desired command group. For example, all USB HID Keyboard
configuration settings are identified with a Tag of KBW.
SubTag: A 3-character case-sensitive field that identifies the desired parameter within the tag group. For example, the
SubTag for the keyboard layout is CTY.
Data: The value for a feature or parameter setting, identified by the Tag and SubTag.
Suffix: “;<ETX>” (HEX: 3B 03), 2 characters.
Multiple commands can be issued within one Prefix/Suffix sequence. For configuration commands, only the Tag, SubTag,
and Data fields must be repeated for each command in sequence. If an additional command is to be applied to the same
Tag, then the command is separated with a comma (,) and only the SubTag and Data fields of the additional commands
are issued. If the additional command requires a different Tag field, the command is separated from previous command by
a semicolon (;).
Responses
Different from command sequence, the prefix of a response consists of the six characters of “<STX><SOH>0000” (HEX:
02 01 30 30 30 30).
The engine responds to serial commands with one of the following three responses:
<ACK> (HEX: 06) Indicates a good command which has been processed.
<NAK> (HEX: 15) Indicates a good configuration command with its Data field entry out of the allowable range for this
Tag and SubTag combination (e.g. an entry for an inter-keystroke delay of 100 when the field will
only allow 2 digits), or an invalid query command.

<ENQ> (HEX: 05) Indicates an invalid Tag or SubTag command.
When responding, the engine echoes back the command sequence with the status character above inserted directly before
each of the punctuation marks (the comma or semicolon) in the command.
Examples
Example 1: Enable Code 11, set the minimum and maximum lengths to 12 and 22 respectively.
Enter: 7E 01 30 30 30 30 40 43 31 31 45 4E 41 31 2C 4D 49 4E 31 32 2C 4D 41 58 32 32 3B 03
 (~<SOH>0000@C11ENA1,MIN12,MAX22;<ETX>)
Response: 02 01 30 30 30 30 40 43 31 31 45 4E 41 31 06 2C 4D 49 4E 31 32 06 2C 4D 41 58 32 32 06 3B 03
 (<STX><SOH>0000@C11ENA1<ACK>,MIN12<ACK>,MAX22<ACK>;<ETX>)
Example 2: Query the current minimum and maximum lengths of Code 11.
Enter: 7E 01 30 30 30 30 40 43 31 31 4D 49 4E 2A 2C 4D 41 58 2A 3B 03
 (~<SOH>0000@C11MIN*,MAX*;<ETX>)
Response: 02 01 30 30 30 30 40 43 31 31 4D 49 4E 31 32 06 2C 4D 41 58 32 32 06 3B 03
 (<STX><SOH>0000@C11MIN12<ACK>,MAX22<ACK>;<ETX>)
