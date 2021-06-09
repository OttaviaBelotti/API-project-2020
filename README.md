# API-project-2020
This project is the *Algoritmi e Principi dell'Informatica* course's final test (Politecnico di Milano, A.Y. 2019/20).
<br>
Evaluation: 30L

## Specs
It consists in a simple text editor that take from the standard input a command (see [commands](#commands)), then accordingly to the latter, a number or multiple strings.

## Commands

| Command | Meaning | Usage example| Usage |
|:---------:|---------|------------|-------|
| `c` | change | ```c 3,7```|The numerical pair identifies the interval of strings that need to be changed or added if they weren't written yet (or a mix of the two options). After this command, strings need to follow.|
| `d` | delete | ```d 1,6```|The numerical pair identifies the interval of strings that need to be deleted. If no string is present ad those index, nothing is deleted; if just a fraction of them is present, it deletes just them.|
| `p` | print | ```p 2,8```|The numerical pair identifies the interval of strings that need to be printed through standard output. If no string is present at those index or just a fraction, see command above.|
| `u` | undo | ```u 20```|The number identifies how many previous operations must be reverted. If it's grater than the revertable operations done till this command is used, then it undo all the possibile operations reeturning to an empty file state.|
| `r` | redo | ```r 12```|The number identifies how many undo done must be reverted. If it's grated than the undos, then reverts to the latest state.|
| `q`| quit | | Leave the text editor.|

After typing in the `c` command, the strings (both new or modified) must follow and have to be separated from each other by the `\n` character. Once the user has finished typing in all the strings, the escaping character `.` (full stop) is expected. 

## Example
