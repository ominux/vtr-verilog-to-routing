# Overview #
VTR uses auto formatting tools included in [Eclipse](http://www.eclipse.org/) to perform whitespace auto-formatting.  This formatting will not change code functionality, nor will it change any variable/function naming.  Only whitespace will be modified.

Code should be formatted using these tools before being committed to the VTR trunk.

# File Types #

## C Files (.c/.h) ##
C code has been formatted using the Eclipse 'K&R [built-in]' formatting style, which is included by default in Eclipse CDT.   Be sure to use the latest version of Eclipse with all updates applied, as past versions of the CDT formatter contain bugs.

Instructions for formatting VPR:
  1. Install Eclipse CDT as a [standalone program](http://www.eclipse.org/cdt/downloads.php), or add CDT to your existing eclipse through the Eclipse Marketplace (Help->Eclipse Marketplace...).
  1. Create a VPR project: File->New->Makefile Project with Existing Code.
  1. Make sure the project references libvpr.  If you have a project for libvpr, indicate the dependence in Project->Properties->Project References.  Alternatively, add the libvpr headers through Project->Properties->C/C++ General->Paths and Symbols->Includes. Without project references to libvpr, Eclipse will think that parts of code have syntax errors, and will not format these sections.
  1. Check formatter settings: Window->Preferences, C/C++->Code Style, and select the profile: **K&R [built-in]**.
  1. Format the Code: Right-click on the SRC directory, Source->Format.

Similar steps can be taken for libvpr, and other C based code.


## Perl Files (.pl) ##
Instructions for formatting Perl scripts:

  1. Install Eclipse EPIC through the Eclipse Marketplace (Help->Eclipse Marketplace...).  You must already have Eclipse installed.
  1. Create a project for your Perl scripts.
  1. Set formatter settings: Window->Preferences, Perl EPIC->Source Formatter.  Additional options should be: `"-nolc -nolq"`
  1. Open code files and select Source->Format Code.

## Python Files (.py) ##
  1. Install Eclipse Pydev through the Eclipse Marketplace (Help->Eclipse Marketplace...).  You must already have Eclipse installed.
  1. Create a project for your Python scripts.
  1. Open code files and select Source->Format Code.