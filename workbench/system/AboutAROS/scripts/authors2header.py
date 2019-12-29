#!/usr/bin/env python3
# -*- coding: iso-8859-15 -*-
# Copyright � 2002-2008, The AROS Development Team. All rights reserved.
# $Id$

import os, sys, string

labels2sids = {
    'Coordination'                   : 'SID_COORDINATION',
    'Evangelism'                     : 'SID_EVANGELISM',
    'HIDD subsystem'                 : 'SID_HIDD',
    'Intuition'                      : 'SID_INTUITION',
    'Graphics'                       : 'SID_GRAPHICS',
    'Shell commands'                 : 'SID_SHELL_COMMANDS',
    'Workbench'                      : 'SID_WORKBENCH',
    'Tools'                          : 'SID_TOOLS',
    'Preferences'                    : 'SID_PREFERENCES',
    'BGUI'                           : 'SID_BGUI',
    'Zune'                           : 'SID_ZUNE',
    'Kernel'                         : 'SID_KERNEL',
    'DOS'                            : 'SID_DOS',
    'Networking'                     : 'SID_NETWORKING',
    'C link library and POSIX layer' : 'SID_LIBC_POSIX',
    'Documentation'                  : 'SID_DOCUMENTATION',
    'Translation'                    : 'SID_TRANSLATION',
    'Art work'                       : 'SID_ARTISTRY',
    'Website'                        : 'SID_WEBSITE',
    'Datatypes'                      : 'SID_DATATYPES',
    'Testing'                        : 'SID_TESTING'
}

def parse(file):
    credits = []
    names   = []

    for line in file:
        line = line.strip()

        if ':' in line:
            if len(names) > 0:
                credits.append([area, names])

            area = line[:-1]

            names = []

        elif line != '':
            names.append(line)

    if len(names) > 0:
        credits.append([area, names])

    return credits

file = open(sys.argv[1], "r", encoding="iso-8859-15")
credits = parse(file)

sys.stdout.write('''#ifndef _AUTHORS_H_
#define _AUTHORS_H_

/*
    Copyright � 2008, The AROS Development Team. All rights reserved.
    ****** This file is automatically generated. DO NOT EDIT! *******
*/

#include <utility/tagitem.h>
#include <zune/aboutwindow.h>

#define SID_COORDINATION    (100)
#define SID_EVANGELISM      (101)
#define SID_HIDD            (102)
#define SID_INTUITION       (103)
#define SID_GRAPHICS        (104)
#define SID_SHELL_COMMANDS  (105)
#define SID_WORKBENCH       (106)
#define SID_TOOLS           (107)
#define SID_PREFERENCES     (108)
#define SID_BGUI            (109)
#define SID_ZUNE            (110)
#define SID_KERNEL          (111)
#define SID_DOS             (112)
#define SID_LIBC_POSIX      (113)
#define SID_DOCUMENTATION   (114)
#define SID_TRANSLATION     (115)
#define SID_ARTISTRY        (116)
#define SID_WEBSITE         (117)
#define SID_DATATYPES       (118)
#define SID_NETWORKING      (119)
#define SID_TESTING         (120)

struct TagItem *AUTHORS = TAGLIST
(
''')

for area in credits:
    sys.stdout.write('''    SECTION
    (
        %s''' % labels2sids[area[0]])
            
    for name in area[1]:
        sys.stdout.write(',\n        NAME("%s")' % name.replace('"', '\\"'))
    
    print('\n    ),')
    
print('''    TAG_DONE
);

#endif /* _AUTHORS_H_ */''')

file.close()
