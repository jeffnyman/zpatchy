# Zpatchy

Zpatchy is a simple repository containing two C utility files that allow patches between different versions of z-code files that run on a Z-Machine.

## Context

The name "Apache" -- as in the web server -- originated from a set of patches that were collected and collated against the original NCSA HTTPd server source code. The name was derived from this original "patch" collection; hence it was an APAtCHy server, which was then simplified to Apache.

In this context, a "patch" refers to a specific collection of differences between files that can be applied to a source code tree using a diff utility. A "diff" is short for "difference;" a program that generates diffs essentially reveals the difference between one or more files.

So we can create diffs (or patches) using a diff tool and then apply them to an unpatched version of that same source code using a patch tool. Interestingly, the word "patch" comes from the physical covering of punchcard holes to make software changes in the early computing days, when punchcards represented the program executed by the computer's processor.

In order to write an Z-Machine emulator, you (ideally) need to be able to check v1 and v2 versions of Z-Machine programs to make sure you are fully compliant with the specification. That being said, it's not easy to any such genuine v1 or v2 programs.

So the main reason the Zpatchy utilities were needed was because it became necessary to take some v3 or v5 files and convert them back to v1 and v2 versions. This is done via the process of diffing and patching.

Specifically, the pair of command line programs (z-differ and z-patcher) will create and apply patches between different versions of a given z-code file, in particularly those from Infocom.

The patch files have a header specifying the name of the game, the version the patch file "decrypts" to, and the the version required to "decrypt" it. It also includes checks to make sure you supply the right data file.

As far as the "encryption" method, the program reads in the file size from the game file header and uses that to determine when to reset back to the start of the encryption file. This is as opposed to using the file's actual size. This means that decryption is not dependent on having the decryption file be exactly the same as the file used for encryption, including having the suffix padding bytes match. Thus the padding on the file you use for decryption should make no difference.

The "encryption" part is called diffing. The "decryption" is called patching.

The reason I put these in quotes is that what's happening is not truly any sort of decryption or encryption. How this process works with these utilities is that a logical exclusion (XOR) is performed on a patch file with a source file. So if you are XORing the patch with a source, you can see that as a form of "decryption." That would mean the opposite process could be seen as something like "encryption."

## Patch File Format

You will often hear of a "game file" in this context. That term translates to a z-code file, which is a binary format of a game that could run on the Z-Machine. More generally, this would be a z-program which doesn't have to be a game.

A patch file will be in the following format:

(1) The patch file will provide a header identifier string: PFG.

(2) The patch file will provide a game name, which can take up to 32 bytes. Any unused bytes will be filled with 0 values.

(3) The patch file will provide a target game file version, taken from the z-code header, which takes up 9 bytes. This will be made up of the z-code version (1 byte), the release number (2 bytes), and the version of the game itself (6 bytes).

(4) The patch file will provide a source game file version, which takes up 9 bytes and matches the format for the target game file version.

(5) Finally, the patch file will contain the XOR data used to translate the game file.

## Compiling and Installing

These programs should compile on any system with a C toolchain available to it, including Windows. If you are on Windows and you plan to use the Makefile, make sure to uncomment the EXTENSION portion.

For any POSIX-based system with an adequate C development chain, you can compile and install as follows:

```
$ make
```

## Usage

z-differ takes two zcode files and creates a patch file, via the following syntax:

```
$ z-differ from.z5 to.z5 from-to.pat
```

z-patcher applies a patch, via the following syntax:

```
$ z-patcher from-to.pat from.z5 to.z5
```

In both of the above examples, the first two files are required and the last file will be created as output.

z-patcher can also give information about a patch, via the following syntax:

```
$ z-patcher from-to.pat
```

Help for each utility can be viewed by running it with no arguments.

Patch files for several Infocom games can be found in here:

http://www.ifarchive.org/indexes/if-archiveXinfocomXpatches.html

## Example

Release 2 and release 5 of Zork are .z1 files and Release 15 is a .z2 file. To patch to these versions requires having a distribution of the Release 88 version of Zork 1. This is a .z5 file and is included with this distribution. I also have a torrent file (`Zork1Release88Z`) available where the file can be retrieved from.

I provided some patch files (in the `patches` directory) to get you started. The most important of those is the `zork1_88_to_2` patch file.

You can apply those patches using the provided program, which will recreate those earlier versions of the game. I'll give you the specific steps here.

### Produce a z1 version ###

```
./z-patcher patches/zork1_88_to_2.pat examples/zork1.z5 /examples/zork1.z1
```

If you don't want to, or can't, run the Makefile, you can use your C compiler to do this directly (all on one line):

```
gcc -Wall z-patcher.c -o z-patcher &&
  ./z-patcher patches/zork1_88_to_2.pat examples/zork1.z5 examples/zork1.z1
```

You should see something like this:

```
Patching game "" 2/AS000C [v1]
using 88/840726 [v3] as source.

Patch applied successfully.
```

### Produce a z2 version ###

```
./z-patcher patches/zork1_88_to_3german.pat examples/zork1.z5 examples/zork1.z1
./z-patcher patches/zork1_3german_to_5.pat examples/zork1.z1 examples/zork1.z
./z-patcher patches/zork1_5_to_15.pat examples/zork1.z examples/zork1.z2
```

The set of output you will get is:

```
Patching game "Zork I - german" 3/880113 [v5]
using 88/840726 [v3] as source.

Patch applied successfully.

Patching game "Zork I-82836.a8a4" 5/$000000000000 [v1]
using 3/880113 [v5] as source.

Patch applied successfully.

Patching game "Zork I-78566.e987" 15/UG3AU5 [v2]
using 5/$000000000000 [v1] as source.

Patch applied successfully.
```

On the third command, you will get a warning of "Source file 'examples/zork1.z' does not have an embedded file size." Don't worry about that.

With those commands, do note that in the second command I make a file called "zork1.z" and I did that just so I don't have to overwrite an existing name in the third command. Both the second and third command patch to a z2 version. But I have to approach this indirectly by using a German patch and then back to English.

As before, you can use the same commands via your compiler as well if the Makefile is not working for you.

## Credits

These utilities are based on zcdiff and zcpatch by Paul Gilbert, Rodney Hester, Nils Barth, and Mike Ciul. Both of these original utilities were distributed under the GNU General Public License, version 3.

The original files can be downloaded from the Interactive Fiction Archive:

http://www.ifarchive.org/if-archive/infocom/patches/zpat-2_3.zip
