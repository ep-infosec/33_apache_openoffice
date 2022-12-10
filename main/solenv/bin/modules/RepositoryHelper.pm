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



#*************************************************************************
#
# RepositoryHelper - Perl for working with repositories and underlying SCM 
#
# usage: see below
#
#************************************************************************* 

package RepositoryHelper;

use strict;


use Carp;
use Cwd qw (cwd);
use File::Basename;
#use File::Temp qw(tmpnam);

my $debug = 0;

#####  profiling #####

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $initial_directory = shift;
    if ($initial_directory) {
        $initial_directory = Cwd::realpath($initial_directory);
    } else {
        if ( defined $ENV{PWD} ) {
            $initial_directory = $ENV{PWD};
        } elsif (defined $ENV{_cwd}) {
            $initial_directory = $ENV{_cwd};
        } else {
            $initial_directory = cwd();
        };
    };
    my $self = {};
    $self->{INITIAL_DIRECTORY} = $initial_directory;
    $self->{REPOSITORY_ROOT} = undef;
    $self->{REPOSITORY_NAME} = undef;
    $self->{SCM_NAME} = undef;
    detect_repository($self);
    bless($self, $class);
    return $self;
}

##### methods #####
sub get_repository_root 
{   
    my $self        = shift;
    return $self->{REPOSITORY_ROOT};
}

sub get_initial_directory 
{
    my $self        = shift;
    return $self->{INITIAL_DIRECTORY};
}

sub get_scm_name 
{
    my $self        = shift;
    return$self->{SCM_NAME}; 
}

##### private methods #####
sub search_for_hg {
    my $self        = shift;
    my $hg_root;
    my $scm_name = 'hg';
    if (open(COMMAND, "$scm_name root 2>&1 |")) {
        foreach (<COMMAND>) {
            next if (/^Not trusting file/);
            chomp;
            $hg_root = $_;
            last;
        };
        close COMMAND;
        chomp $hg_root;
        if ($hg_root !~ /There is no Mercurial repository here/) {
            $self->{REPOSITORY_ROOT} = $hg_root;
            $self->{SCM_NAME} = $scm_name;
            return 1;
        };
    };
    return 0;
};

sub search_via_build_lst {
    my $self = shift;
#    my @possible_build_lists = ('build.lst', 'build.xlist'); # build lists names
    my @possible_build_lists = ('build.lst'); # build lists names
    my $previous_dir = '';
    my $rep_root_candidate = $self->{INITIAL_DIRECTORY};
    do {
        foreach (@possible_build_lists) {
            my $test_file;
            if ($rep_root_candidate eq '/') {
                $test_file = '/prj/' . $_;
            } else {
                $test_file = $rep_root_candidate . '/prj/' . $_;
            };
            if (-e $test_file) {
                $self->{REPOSITORY_ROOT} = File::Basename::dirname($rep_root_candidate);
                return 1;
            };
        };
        $previous_dir = $rep_root_candidate;
        $rep_root_candidate = File::Basename::dirname(Cwd::realpath($rep_root_candidate));
        return 0 if ((!$rep_root_candidate) || ($rep_root_candidate eq $previous_dir));
    }
    while (chdir "$rep_root_candidate");
};

sub detect_repository {
    my $self        = shift;
    return if (search_via_build_lst($self));
    chdir $self->{INITIAL_DIRECTORY};
    return if (search_for_hg($self));
    croak('Cannot determine source directory/repository for ' . $self->{INITIAL_DIRECTORY});
};

##### finish #####

1; # needed by use or require

__END__

=head1 NAME

RepositoryHelper - Perl module for working with repositories and underlying SCM

=head1 SYNOPSIS

    # example that will analyze sources and return the source root directory

    use RepositoryHelper;

    # Create a new instance:
    $a = RepositoryHelper->new();

    # Get repositories for the actual workspace:
    $a->get_repository_root();

  
=head1 DESCRIPTION

RepositoryHelper is a perlPerl module for working with repositories and underlying SCM 
in the database.

Methods:

RepositoryHelper::new()

Creates a new instance of RepositoryHelper. Can be initialized by: some path which likely to belong to a repository, default - empty, the current dir will be taken.

RepositoryHelper::get_repository_root()

Returns the repository root, retrieved by SCM methods or on educated guess...

RepositoryHelper::get_initial_directory()

Returns full path to the initialistion directory.

=head2 EXPORT

RepositoryHelper::new()
RepositoryHelper::get_repository_root()
RepositoryHelper::get_scm_name()
RepositoryHelper::get_initial_directory()

=head1 AUTHOR

Vladimir Glazunov, vg@openoffice.org

=head1 SEE ALSO

perl(1).

=cut
