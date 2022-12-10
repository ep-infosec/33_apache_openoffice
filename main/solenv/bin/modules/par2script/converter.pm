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




package par2script::converter;

use par2script::remover;

#############################
# Converter
#############################

sub convert_array_to_hash
{
	my ($arrayref) = @_;  

	my ($line, $key, $value);
	
	my %newhash = ();

	for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
	{
		$line = ${$arrayref}[$i];
		
		if ( $line =~ /^\s*(\w+?)\s+(.*?)\s*$/ )
		{
			$key = $1;
			$value = $2;
			$newhash{$key} = $value;
		}
	}

	return \%newhash;
}

sub convert_hash_into_array
{
	my ($hashref) = @_;
	
	my @array = ();
	my ($key, $value, $input);
	
	foreach $key (keys %{$hashref})
	{
		$value = $hashref->{$key};
		$input = "$key = $value\n";
		push(@array ,$input);
	}

	return \@array
}

sub convert_stringlist_into_array_2
{
	my ( $input, $separator ) = @_;
	
	my @newarray = ();
	my $first = "";
	my $last = "";

	$last = $input;

	while ( $last =~ /^\s*(.+?)\s*\Q$separator\E\s*(.+)\s*$/)	# "$" for minimal matching
	{
		$first = $1;
		$last = $2;
		par2script::remover::remove_leading_and_ending_whitespaces(\$first);
		if ( $first ) { push(@newarray, $first); }
	}	

	par2script::remover::remove_leading_and_ending_whitespaces(\$last);
	if ( $last ) { push(@newarray, $last); }	
	
	return \@newarray;
}

sub convert_stringlist_into_array
{
	my ( $includestringref, $separator ) = @_;
	
	my @newarray = ();
	my ($first, $last);

	$last = ${$includestringref};

	while ( $last =~ /^\s*(.+?)\s*\Q$separator\E\s*(.+)\s*$/)	# "$" for minimal matching
	{
		$first = $1;
		$last = $2;
		par2script::remover::remove_leading_and_ending_whitespaces(\$first);
		push(@newarray, $first);
	}	

	par2script::remover::remove_leading_and_ending_whitespaces(\$last);
	push(@newarray, $last);	
	
	return \@newarray;
}

#############################################################################
# The file name contains for some files "/". If this programs runs on 
# a windows platform, this has to be converted to "\".
#############################################################################

sub convert_slash_to_backslash
{
	my ($filesarrayref) = @_;

	my ($onefile, $filename);

	for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
	{
		$onefile = ${$filesarrayref}[$i];
		$onefile->{'Name'} =~ s/\//\\/g;
	}
}

1;
