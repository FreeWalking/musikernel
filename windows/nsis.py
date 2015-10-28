# Run this script from Windows, not MSYS2
"""
This file is part of the MusiKernel project, Copyright MusiKernel Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import os
import shutil
import subprocess

TEMPLATE = r"""; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "MusiKernel"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "MusiKernel Team"

;Require admin rights on NT6+ (When UAC is turned on)
RequestExecutionLevel admin

SetCompressor /SOLID lzma

Name "${{PRODUCT_NAME}} ${{PRODUCT_VERSION}}"
OutFile "{MAJOR_VERSION}-{MINOR_VERSION}-win-x{bits}.exe"
InstallDir "C:\{MAJOR_VERSION}\mingw{bits}"

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING
  !define MUI_LICENSEPAGE_CHECKBOX

!include MUI2.nsh

;--------------------------------
;Modern UI Configuration
;Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gpl-3.0.txt"
;!insertmacro MUI_PAGE_COMPONENTS
;!insertmacro MUI_PAGE_DIRECTORY
;!insertmacro MUI_PAGE_STARTMENU pageid variable
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
;!insertmacro MUI_UNPAGE_LICENSE textfile
;!insertmacro MUI_UNPAGE_COMPONENTS
;!insertmacro MUI_UNPAGE_DIRECTORY
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"

Section "install"
  RMDir /r $INSTDIR
  SetOutPath $INSTDIR
  writeUninstaller "$INSTDIR\uninstall.exe"
  File /r "C:\{MAJOR_VERSION}\mingw{bits}\*"
  RMDir /r "$SMPROGRAMS\${{PRODUCT_NAME}} ({bits} bit)"
  CreateDirectory "$SMPROGRAMS\${{PRODUCT_NAME}} ({bits} bit)"
  SetOutPath "$INSTDIR\bin"
  createShortCut \
    "$SMPROGRAMS\${{PRODUCT_NAME}} ({bits} bit)\${{PRODUCT_NAME}} ({bits} bit).lnk" \
    "$INSTDIR\bin\{MAJOR_VERSION}.bat" "" \
    "$INSTDIR\{MAJOR_VERSION}.ico" "" SW_SHOWMINIMIZED
SectionEnd

Section "uninstall"
  RMDir /r $INSTDIR
  RMDir /r "$SMPROGRAMS\${{PRODUCT_NAME}} ({bits} bit)"
  ; deprecated
  RMDir /r "$SMPROGRAMS\${{PRODUCT_NAME}}"
SectionEnd
"""

CWD = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(CWD, "..", "src", "minor-version.txt")) as fh:
    MINOR_VERSION = fh.read().strip()

with open(os.path.join(CWD, "..", "src", "major-version.txt")) as fh:
    MAJOR_VERSION = fh.read().strip()

#mingw-w64-i686-portaudio-19_20140130-2-any.pkg.tar.xz

shutil.copy(
    os.path.join(CWD, "mingw-w64-portaudio",
        "mingw-w64-x86_64-portaudio-19_20140130-2-any.pkg.tar.xz"),
    r"C:\{}\home\pydaw".format(MAJOR_VERSION))

for arch, bits in (("x86_64", "64"),): # ("i686", "32"),
    src = ("mingw-w64-{arch}-{MAJOR_VERSION}-{MINOR_VERSION}"
        "-1-any.pkg.tar.xz".format(
        arch=arch, MAJOR_VERSION=MAJOR_VERSION, MINOR_VERSION=MINOR_VERSION))
    dest = r"C:\{MAJOR_VERSION}\home\pydaw".format(
        bits=bits, MAJOR_VERSION=MAJOR_VERSION)
    if not os.path.isdir(dest):
        os.makedirs(dest)
    shutil.copy(src, dest)

shell = r"C:\{}\mingw64_shell.bat".format(MAJOR_VERSION)
os.system(shell)

print("""\
#In the terminal, run:
pacman -U mingw-[version]' # for each package
rm *  # remove all package files to save space
""")
input("Press 'enter' to continue")

shutil.copy(
    os.path.join(CWD, "{}.bat".format(MAJOR_VERSION)),
    r"C:\{}\mingw64\bin".format(MAJOR_VERSION))

NSIS = r"C:\Program Files (x86)\NSIS\Bin\makensis.exe"

for bits in ("64",): # "32",
    template = TEMPLATE.format(
        bits=bits, MINOR_VERSION=MINOR_VERSION, MAJOR_VERSION=MAJOR_VERSION)
    template_name = "{0}-{1}.nsi".format(MAJOR_VERSION, bits)
    with open(template_name, "w") as fh:
        fh.write(template)
    subprocess.call([NSIS, template_name])
