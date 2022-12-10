:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
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


#
# add keyids to sdf file
#

use Compress::Zlib();

print "\nkeyidGen version 1.0 \n\n";

my ( $infile,$outfile,$dbimport );
get_options();

print_help() if ( !defined $infile || $help );
exit 1 if ( !defined $infile );
if ( ! defined $outfile )
{
    $outfile = $infile;
    $outfile =~ s/\.sdf$//i;
    $outfile .= "_KeyID.sdf";
}

$collisions = 0;
%hashcodes = ();
$count = 0;
print "writing to $outfile\n";
open INFILE,"<$infile" || die "could not open $infile $! $^E\n";
open OUTFILE,">$outfile" || die "could not open $outfile $! $^E\n";

while ( <INFILE> )
{
    $line = $_;
    chomp $line;
    $hash = 0;
    if ( $line =~ /^([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)$/ )
    {
        $string="$1 $2 $4 $5 $6 $7 $8";
        $hashp = makeID( $string );

        if ( defined ($hashcodes{ $hashp } ) )
        {
            $collisions ++ unless $hashcodes{ $hashp } eq $string;
        }
        $hashcodes{ $hashp } = $string;
        $count++;
        if ( $dbimport )
        {
            my ( $pre, $post, $old );
            $pre = "$1\t$2\t";
            $post = "\t$4\t$5\t$6\t$7\t$8\t$9\t$10\t$11\t$12\t$13\t$14\t$15\n";
            $old = $3;
            $old =~ s/;{0,1}keyid:......;{0,1}//;
            $old =~ s/^0$//;
            if ( $old ne "" ) { $old .= ";"; }
            print OUTFILE "$pre${old}keyid:$hashp$post";
        }
        else
        {
            print OUTFILE "$1\t$2\t$3\t$4\t$5\t$6\t$7\t$8\t$9\t$10\t".makekidstr($hashp,$11)."\t".makekidstr($hashp,$12)."\t$13\t".makekidstr($hashp,$14)."\t$15\n";
        }
    }
}
print "$count entries\n";
print "$collisions collisions\n";

close INFILE;
close OUTFILE;

sub makeID
{
	my ( $String ) = shift;
	my ( $hash );
	# hardcoded to prevent windows installer to choke on bad directoryname :-((
	if ( $String eq "scp2 source\\ooo\\directory_ooo.ulf LngText STR_DIR_KAPITEL   " )
	{
		return "keyid1";
	}

    $hash = Compress::Zlib::crc32( $String, undef );
    return makenumber( $hash );
}

sub makenumber
{
    $h = shift;
    #                  1         2         3         4
    #         1234567890123456789012345678901234567890
    $symbols="0123456789abcdefghijklmnopqrstuvwxyz+-[=]";
    $order = length($symbols);
    $result = "";
    while ( length( $result ) < 6 )
    {
        $result .= substr( $symbols, ($h % $order), 1 );
        $h = int( $h / $order );
    }
    die "makenumber failed because number is too big (this cannot be so this is a strange error)" if $h > 0;
    
    return reverse $result;
}


sub makekidstr
{
    $kid = shift;
    $str = shift;
    
    if ( $str ne "" )
    {
        # special handling for strings starting with font descriptions like {&Tahoma8} (win system integration)
        if ( $str =~ s/^(\{\&[^\}]+\})// )
        {
            return "$1$kid‖$str";
        }
        else
        {
            return "$kid‖$str";
        }
    }
    else
    {
        return "";
    }
#    return "default";
}

sub print_help
{
    print "\n\n";
    print "keyidGen 0.5 for sdf files\n";
    print "--------------------------\n";
    print "Usage:\n";
    print "keyidGen <infile> [<outfile>] [-dbimport]\n";
    print "                   add keyids to the entries and write them to a file with\n";
    print "                   _KeyID added to the name\n";
    print "   -dbimport       Add KeyID to a new column instead of to the strings.\n";
    print "                   This is needed to import the IDs into tha database.\n";
    print "\n\n";
}


sub get_options {
	my ($arg,$has_infile);

	while ($arg = shift @ARGV) {
		$arg =~ /^-dbimport$/  and $dbimport = 1 and next;
		$arg =~ /^-help$/  and $help = 1 and next; #show help
		
		if ( !$has_infile )
		{
		    $infile = $arg;
		    $has_infile = 1;
		}
		else
		{
		    $outfile = $arg;
		}
	}
}
