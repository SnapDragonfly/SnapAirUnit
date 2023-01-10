# SnapAirUnit

It's an education version of Snap Air Unit.

*Note：md file is using Markdown language，details about how to, please check following documents：*
1. [Markdown language used in github project](https://blog.csdn.net/lida2003/article/details/127828153)
2. [MarkDown tutorial](https://www.runoob.com/markdown/md-tutorial.html)

# 

How to seedup work progress：issues + tags

## Issue Tags

**The project involves a wide range of types of work (hardware, software, testing, etc.). In order to better coordinate work and form phased achievements, GitHub issue+tag is used to form various work contents. See the following classification for details：**

* Task：Tasks involving SnapAirUnit project code (such as: baseline version, compilation, construction and burning guide, etc.)
* Feature：New features (such as new UDP forward x instructions)
* Enhancement：Function optimization (for example, the algorithm changes from O (n) to O (1), etc.)
* Bug：Problems found in the process of testing and using (for example, the mobile phone controls the forward movement, and the aircraft moves as expected, etc.)

## Task Description

**To quickly understand the task, the following tags are used for simple description：**

* documentation：It is recommended that you use md syntax for document work.
* duplicate：For repeated tasks, please R&D directly annotate the repeated task number and assign it to the demand dispatching personnel
* good first issue：--- bla..bla.. ---
* help wanted：If you need help, please annotate the content you need and assign it to the person who needs it.
* invalid：Invalid task, please R&D annotate the invalid reason and assign it to the demand dispatcher
* question：If there are problems (unclear requirements, doubtful points or design conflicts), please make comments after the R&D analyzes the problems and assign them to the personnel assigned by the requirements.
* wontfix：It is usually marked after the requirement dispatcher coordinates, discusses and makes decisions, and comments rootcause.

## Git Code Commit

**In order to better understand the code and compare and analyze the code, please pay attention to the following work each time you submit the code：**

1. Please use English instead of Chinese characters for notes.
2. Please be brief and comprehensive, and mark the issue number with "[# 2]" at the front of the note.
3. The content of notes is complex, and there is no corresponding issue. Please develop your own issue according to the technical content.

*Note: The details of the work determine the orderliness of the subsequent debugging analysis, to ensure that the root cause can be analyzed, traced and found, and that the research and development should be down-to-earth and step by step.*

# Project

## Coding rule
* After the baseline version, please use English for all comments of the new code // Prevent some compilers and IDE tools from handling Chinese characters well
* The coding specification shall be the unified specification of the company. If the company does not enforce it, the development team shall build its own specification and update it to the document // It is recommended to consider maintaining the SDK baseline version coding style

## VersionControl
[Software version management principles](https://blog.csdn.net/lida2003/article/details/36617839)
Version control specification：x.y.commitid-clean/dirty/release
* x: Major version number, maintained by the project leader
* y: The minor version number is maintained by the version manager
* commitid: The first 7 digits, such as the following initial version，commit id = 39034f8
* clean/dirty/release: Clean (consistent with git version), dirty (local version), release (release version)


> commit b42cee176eb4a7592c29d2c04fcee09b11e6be18 (HEAD -> main, origin/main, origin/HEAD)
>
> Author: Daniel Li <lida_mail@163.com>
>
> Date:   Fri Dec 23 14:17:57 2022 +0800
>   
>  
>    [#51] Add wireless get/post RESTful API


# Document

     .
    ├──> docs  //images used in *.md documents
    │   ├──> images
    │   ├──> Build_and_Flash.md
    │   ├──> How_to_Use.md
    │   └──> How_to_Connect.md
    └──> README.md

## 1. Compile and build firmware method

Details：[Build_and_Flash.md](./docs/Build_and_Flash.md)

## 2. How to connect hardware to flight control

Details：[How_to_Connect.md](./docs/How_to_Connect.md)

## 3. How to use SnapAirUnit

Details：[How_to_Use.md](./docs/How_to_Use.md)

