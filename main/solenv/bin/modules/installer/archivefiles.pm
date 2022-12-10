#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



package installer::archivefiles;

use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;

#################################################################
# Changing the name for files with flag RENAME_TO_LANGUAGE
#################################################################

sub put_language_into_name
{
	my ( $oldname, $onelanguage ) = @_;
	
	my $newname = "";
	
	my $filename = "";
	my $extension = "";
	
	if ( $oldname =~ /en-US/ )	# files, that contain the language in the file name
	{
		$newname = $oldname;
		$newname =~ s/en-US/$onelanguage/;
	}
	else	# files, that do not contain the language in the file name
	{
		if ( $oldname =~ /^\s*(.*)(\..*?)\s*$/ )	# files with extension
		{
			$filename = $1;
			$extension = $2;	
		}
		else
		{
			$filename = $oldname;
			$extension = "";	
		}
	
		$newname = $1 . "_" . $onelanguage . $2;
	}
	
	return $newname;
}

#################################################################
# Converting patchfiles string into array
#################################################################

sub get_patch_file_list
{
	my ( $patchfilestring ) = @_;
	
	$patchfilestring =~ s/^\s*\(?//;
	$patchfilestring =~ s/\)?\s*$//;
	$patchfilestring =~ s/^\s*\///;
	$patchfilestring =~ s/^\s*\\//;
	
	my $patchfilesarray = installer::converter::convert_stringlist_into_array_without_linebreak_and_quotes(\$patchfilestring, ",");

	return $patchfilesarray;
}

#################################################################
# Reading all executables in the "manifest.xml"
#################################################################

sub get_all_executables_from_manifest
{
	my ($unzipdir, $manifestfile, $executable_files_in_extensions) = @_;
	
	my $is_executable = 0;
	
	for ( my $i = 0; $i <= $#{$manifestfile}; $i++ )
	{
		my $line = ${$manifestfile}[$i];
		
		if ( $line =~ /\"application\/vnd\.sun\.star\.executable\"/ ) { $is_executable = 1; }

		if (( $line =~ /manifest\:full\-path=\"(.*?)\"/ ) && ( $is_executable ))
		{
			my $filename = $unzipdir . $installer::globals::separator . $1;
			# making only slashes for comparison reasons
			$filename =~ s/\\/\//g;
			$executable_files_in_extensions->{$filename} = 1;
		}

		if ( $line =~ /\/\>/ ) { $is_executable = 0; }
	}
}

#################################################################
# Reading the "manifest.xml" in extensions and determine, if
# there are executable files
#################################################################

sub collect_all_executable_files_in_extensions
{
	my ($unzipdir, $executable_files_in_extensions) = @_;
	
	$unzipdir =~ s/\Q$installer::globals::separator\E\s*$//;
		
	my $manifestfilename = $unzipdir . $installer::globals::separator . "META-INF" . $installer::globals::separator . "manifest.xml";

	if ( -f $manifestfilename )
	{
		my $manifestfile = installer::files::read_file($manifestfilename);
		get_all_executables_from_manifest($unzipdir, $manifestfile, $executable_files_in_extensions);		
	}
}

#################################################################
# Analyzing files with flag ARCHIVE
#################################################################

sub resolving_archive_flag
{
	my ($filesarrayref, $additionalpathsref, $languagestringref, $loggingdir) = @_;

	if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::archivefiles::resolving_archive_flag : $#{$filesarrayref} : $#{$additionalpathsref} : $$languagestringref : $loggingdir"); }
	
	my @newallfilesarray = ();

	my ($systemcall, $returnvalue, $infoline);
		
	my $unziplistfile = $loggingdir . "unziplist_" . $installer::globals::build . "_" . $installer::globals::compiler . "_" . $$languagestringref . ".txt";

	my $platformunzipdirbase = installer::systemactions::create_directories("zip", $languagestringref);
	push(@installer::globals::removedirs, $platformunzipdirbase);

	installer::logger::include_header_into_logfile("Files with flag ARCHIVE:");

	my $repeat_unzip = 0;
	my $maxcounter = 0;

	for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
	{
		if ( $repeat_unzip ) { $i--; }	# decreasing the counter

		my $onefile = ${$filesarrayref}[$i];
		my $styles = "";
		
		if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
	
		if ( $styles =~ /\bARCHIVE\b/ )		# copying, unzipping and changing the file list
		{
			my $iscommonfile = 0;			
			my $sourcepath = $onefile->{'sourcepath'};			

			if ( $sourcepath =~ /\Q$installer::globals::separator\E\bcommon$installer::globals::productextension\Q$installer::globals::separator\E/ )	# /common/ or /common.pro/
			{
				$iscommonfile = 1;
			}

			my $use_internal_rights = 0;
			if ( $styles =~ /\bUSE_INTERNAL_RIGHTS\b/ ) { $use_internal_rights = 1; }	# using the rights used inside the zip file

			my $rename_to_language = 0;			
			if ( $styles =~ /\bRENAME_TO_LANGUAGE\b/ ) { $rename_to_language = 1; }	# special handling for renamed files (scriptitems.pm)

			my %executable_files_in_extensions = ();
			my $set_executable_privileges = 0;  # setting privileges for exectables is required for oxt files
			if ( $onefile->{'Name'} =~ /\.oxt\s*$/ ) { $set_executable_privileges = 1; }					

			# mechanism to select files from an archive files
			my $select_files = 0;
			my $selectlistfiles = "";
			my @keptfiles = ();
			if ( $onefile->{'Selectfiles'} )
			{
				$select_files = 1;
				$selectlistfiles = get_patch_file_list( $onefile->{'Selectfiles'} );
                $installer::logging::Lang->printf("Selected file list defined at file: %s :\n", $onefile->{'Name'});
                foreach my $line (@$selectlistfiles)
				{
                    $installer::logging::Lang->printf("\"%s\"\n", $line);
				}
			}
			
			if ( $onefile->{'Selectfiles'} ) { $onefile->{'Selectfiles'} = ""; } # Selected files list no longer required

			# mechanism to define patch files inside an archive files
			my $select_patch_files = 0;
			my $patchlistfiles = "";
			my @keptpatchflags = ();
			if (( $styles =~ /\bPATCH\b/ ) && ( $onefile->{'Patchfiles'} ) && ( $installer::globals::patch ))
			{
				$select_patch_files = 1; # special handling if a Patchlist is defined
				$patchlistfiles = get_patch_file_list( $onefile->{'Patchfiles'} );
                $installer::logger::Lang->printf("Patch file list defined at file: %s :\n", $onefile->{'Name'});
				foreach my $line (@$patchlistfiles)
				{
                    $installer::logger::Lang->printf("\"%s\"\n", $line);
				}
			}

			if ( $onefile->{'Patchfiles'} ) { $onefile->{'Patchfiles'} = ""; } # Patch file list no longer required
			
			# creating directories

			my $onelanguage = $onefile->{'specificlanguage'};
			
			# files without language into directory "00"
			
			if ($onelanguage eq "") { $onelanguage = "00"; }

			my $unzipdir;

			# if ($iscommonfile) { $unzipdir = $commonunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator; }
			# else { $unzipdir = $platformunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator; }

			$unzipdir = $platformunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;
			
			installer::systemactions::create_directory($unzipdir);	# creating language specific subdirectories

			my $onefilename = $onefile->{'Name'};
			$onefilename =~ s/\./\_/g;		# creating new directory name
			$onefilename =~ s/\//\_/g;		# only because of /letter/fontunxpsprint.zip, the only zip file with path
			$unzipdir = $unzipdir . $onefilename . $installer::globals::separator; 

			if ( $installer::globals::dounzip ) { installer::systemactions::create_directory($unzipdir); }	# creating subdirectories with the names of the zipfiles

			my $zip = Archive::Zip->new();
			if ( $zip->read($sourcepath) != AZ_OK )
			{
                $installer::logger::Lang->printf("ERROR: Could not unzip %s\n", $sourcepath);
			}

			my $counter = 0;
			my $contains_dll = 0;
			foreach my $member ( $zip->memberNames() )
			{
				$counter++;
				if ( $member =~ /.dll\s*$/ ) { $contains_dll = 1; }
			}

			if (! ( $counter > 0 ))	# the zipfile is empty
			{
                $installer::logger::Lang->printf("ERROR: Could not unzip %s\n", $sourcepath);
			}
			else
			{
				if ( $installer::globals::dounzip )			# really unpacking the files 
				{					
					if ( $zip->extractTree("", $unzipdir) != AZ_OK )
                    {
                        installer::exiter::exit_program("ERROR: can not unzip ".$sourcepath, "resolving_archive_flag");
                    }

					if (( $^O =~ /cygwin/i ) && ( $contains_dll ))
					{
						# Make dll's executable
						$systemcall = "cd $unzipdir; find . -name \\*.dll -exec chmod 775 \{\} \\\;";
						$returnvalue = system($systemcall);
						$installer::logger::Lang->printf("Systemcall: %s\n", $systemcall);
		
						if ($returnvalue)
						{
							$installer::logger::Lang->printf("ERROR: Could not execute \"\"!\n", $systemcall);
						}
					}

					if ( ! $installer::globals::iswindowsbuild )
					{
						# Setting unix rights to "775" for all created directories inside the package
		
						$systemcall = "cd $unzipdir; find . -type d -exec chmod 775 \{\} \\\;";
						$returnvalue = system($systemcall);
                        $installer::logger::Lang->printf("Systemcall: %s\n", $systemcall);
						if ($returnvalue)
						{
                            $installer::logger::Lang->printf("ERROR: Could not execute \"\"!\n", $systemcall);
						}
					}
					
					# Selecting names of executable files in extensions
					if ( $set_executable_privileges )
					{
						collect_all_executable_files_in_extensions($unzipdir, \%executable_files_in_extensions);
					}
				}
	
				my $zipfileref = \@zipfile;
				my $unziperror = 0;
				
				foreach my $zipname ( $zip->memberNames() )
				{
					# Format from Archive:::Zip : 
					# dir1/
					# dir1/so7drawing.desktop

					# some directories and files (from the help) start with "./simpress.idx"
						
					$zipname =~ s/^\s*\.\///;
						
					if ($installer::globals::iswin and $^O =~ /MSWin/i) { $zipname =~ s/\//\\/g; }

					if ( $zipname =~ /\Q$installer::globals::separator\E\s*$/ )	# slash or backslash at the end characterizes a directory
					{
						$zipname = $zipname . "\n";
						push(@{$additionalpathsref}, $zipname);
								
						# Also needed here:
						# Name
						# Language
						# ismultilingual
						# Basedirectory
							
						# This is not needed, because the list of all directories for the 
						# epm list file is generated from the destination directories of the
						# files included in the product!	
					}
					else
					{
						my %newfile = ();
						%newfile = %{$onefile};
						$newfile{'Name'} = $zipname;
						my $destination = $onefile->{'destination'};
						installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
						$newfile{'destination'} = $destination . $zipname;
						$newfile{'sourcepath'} = $unzipdir . $zipname;
						$newfile{'zipfilename'} = $onefile->{'Name'};
						$newfile{'zipfilesource'} = $onefile->{'sourcepath'};
						$newfile{'zipfiledestination'} = $onefile->{'destination'};

						if (( $use_internal_rights ) && ( ! $installer::globals::iswin ))
						{
							my $value = sprintf("%o", (stat($newfile{'sourcepath'}))[2]);
							$newfile{'UnixRights'} = substr($value, 3);
                            $installer::logger::Lang->printf(
                                "Setting unix rights for \"%s\" to \"%s\"\n",
                                $newfile{'sourcepath'},
                                $newfile{'UnixRights'});
						}
						
						if ( $set_executable_privileges )
						{
							# All paths to executables are saved in the hash %executable_files_in_extensions
							my $compare_path = $newfile{'sourcepath'};
							$compare_path =~ s/\\/\//g;  # contains only slashes for comparison reasons
							if ( exists($executable_files_in_extensions{$compare_path}) )
							{
								$newfile{'UnixRights'} = "775";
                                $installer::logger::Lang->printf(
                                    "Executable in Extension: Setting unix rights for \"%s\" to \"%s\"\n",
                                    $newfile{'sourcepath'},
                                    $newfile{'UnixRights'});
							}
						}
						
						if ( $select_files )
						{
							if ( ! installer::existence::exists_in_array($zipname,$selectlistfiles) )
							{
                                $installer::logger::Lang->printf("Removing from ARCHIVE file %s: %s\n",
                                    $onefilename,
                                    $zipname);
								next; # ignoring files, that are not included in $selectlistfiles
							}
							else
							{
                                $installer::logger::Lang->printf("Keeping from ARCHIVE file %s: \n",
                                    $onefilename,
                                    $zipname);
								push( @keptfiles, $zipname); # collecting all kept files
							}			
						}
						
						if ( $select_patch_files )
						{
							# Is this file listed in the Patchfile list?
							# $zipname (filename including path in zip file has to be listed in patchfile list
							
							if ( ! installer::existence::exists_in_array($zipname,$patchlistfiles) )
							{
								$newfile{'Styles'} =~ s/\bPATCH\b//;	# removing the flag PATCH
								$newfile{'Styles'} =~ s/\,\s*\,/\,/;
								$newfile{'Styles'} =~ s/\(\s*\,/\(/;
								$newfile{'Styles'} =~ s/\,\s*\)/\)/;
							}
							else
							{
								push( @keptpatchflags, $zipname); # collecting all PATCH flags 
							}
						}
							
						if ( $rename_to_language )
						{
							my $newzipname = put_language_into_name($zipname, $onelanguage);
							my $oldfilename = $unzipdir . $zipname;
							my $newfilename = $unzipdir . $newzipname;

							installer::systemactions::copy_one_file($oldfilename, $newfilename);

							$newfile{'Name'} = $newzipname;
							$newfile{'destination'} = $destination . $newzipname;
							$newfile{'sourcepath'} = $unzipdir . $newzipname;

                            $installer::logger::Lang->printf("RENAME_TO_LANGUAGE: Using %s instead of %s!\n",
                                $newzipname,
                                $zipname);
						}
							
						my $sourcefiletest = $unzipdir . $zipname;							
						if ( ! -f $sourcefiletest )
						{
                            $installer::logger::Lang->printf("ATTENTION: Unzip failed for %s!\n", $sourcefiletest);
							$unziperror = 1;
						}

						# only adding the new line into the files array, if not in repeat modus

						if ( ! $repeat_unzip ) { push(@newallfilesarray, \%newfile); }			
					}		
				}

				# Comparing the content of @keptfiles and $selectlistfiles
				# Do all files from the list of selected files are stored in @keptfiles ?
				# @keptfiles contains only files included in $selectlistfiles. But are all
				# files from $selectlistfiles included in @keptfiles?
				
				if ( $select_files )
				{
                    $installer::logger::Lang->printf("SELECTLIST: Number of files in file selection list: %d\n",
                        scalar @$selectlistfiles);
                    $installer::logger::Lang->printf("SELECTLIST: Number of kept files: %d\n",
                        scalar @keptfiles);

					foreach my $name (@keptfiles)
					{
                        $installer::logger::Lang->printf("KEPT FILES: %s\n", $name);
					}

					foreach my $name (@$selectlistfiles)
					{
						if ( ! installer::existence::exists_in_array($name,\@keptfiles) )
						{
                            $installer::logger::Lang->printf(
                                "WARNING: %s not included in install set (does not exist in zip file)!\n",
                                $name);;
						}
					}
				}

				# Comparing the content of @keptpatchflags and $patchlistfiles
				# Do all files from the patch list have a PATCH flag ?
				# @keptpatchflags contains only files included in $patchlistfiles. But are all
				# files from $patchlistfiles included in @keptpatchflags?

				if ( $select_patch_files )
				{
                    $installer::logger::Lang->printf("PATCHLIST: Number of files in patch list: %d\n",
                        scalar @$patchlistfiles);
                    $installer::logger::Lang->printf("PATCHLIST: Number of kept PATCH flags: %d\n",
                        scalar @keptpatchflags);

					foreach my $flag (@keptpatchflags)
					{
                        $installer::logger::Lang->printf("KEPT PATCH FLAGS: %s\n",
                            $flag);
					}

					foreach my $name (@$patchlistfiles)
					{
						if ( ! installer::existence::exists_in_array($name,\@keptpatchflags) )
						{
                            $installer::logger::Lang->printf(
                                "WARNING: %s did not keep PATCH flag (does not exist in zip file)!\n",
                                $name);
                        }
					}		
				}

				if ( $unziperror )
				{ 
					installer::logger::print_warning( "Repeating to unpack $sourcepath! \n" );
                    $installer::logger::Lang->printf("ATTENTION: Repeating to unpack %s!\n", $sourcepath);
					$repeat_unzip = 1;
					$maxcounter++;
					
					if ( $maxcounter == 5 )	# exiting the program
					{
						installer::exiter::exit_program("ERROR: Failed to unzip $sourcepath !", "resolving_archive_flag");					
					}				
				}
				else
				{
                    $installer::logger::Lang->printf("Info: %s unpacked without problems !\n", $sourcepath);
					$repeat_unzip = 0;
					$maxcounter = 0;
				}				
			}
		}
		else		# nothing to do here, no zipped file (no ARCHIVE flag)
		{
			push(@newallfilesarray, $onefile);
		}	
	}	

    $installer::logger::Lang->print("\n");

	return \@newallfilesarray;
}


1;
