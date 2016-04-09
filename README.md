# myar_class_project
cs311 Operating Systems class project. Program myar is written in C in Unix and mirrors the AR command.

This program illustrates the use of file I/O on UNIX by managing a UNIX archive library, in the standard archive format.

Usage syntax:
  myar switch archive-file [ member files [ ... ]]

The archive-file is the name of the archive file to be used, and switch is one of the following options (only a single key can be on
the command line).

-q 
“Quickly” append named files (members) to archive. If –q is used and no file members are on the command line, an archive file with no members will be created, just like ar does.

-x
Extract named members. Just as the regular ar command, if no member is named on the command line when extracting files, all files are extracted from the archive. The permissions on the extracted files should match permissions on the files before archiving. To
“extract” a file is not to remove the file from the archive, it is to make a copy of the file outside of the archive file.

-t
Print a concise table of contents of the archive. The concise table of contents for your application (myar) should match exactly the output from the “ar t” command on the same archive file.

-v
Print a verbose table of contents of the archive. The verbose table of contents for your application (myar) should match exactly the output from the “ar tv” command on the same archive file.

-d
Delete named files from archive. To delete a file from the archive does remove it from the archive. If the –d key is used on the command line without any members, the archive file is unaltered.

-A
Quickly append all “regular” files in the current directory (Except the archive itself). There is not an option for the Unix ar command that does this. The -A key is used without any members listed on the command line. If a member is on the command line with the -A key, issue a warning that it is ignored.

-w
For a given timeout (in seconds), add all modified files to the archive (Except the archive itself). There is not an option for the Unix ar command that does this. 
