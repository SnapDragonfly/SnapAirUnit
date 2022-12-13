# SnapAirUnit

It's an education version of Snap Air Unit.

注：md文件使用Markdown语法，详细请参考以下文档：
1. [Github工程中的Markdown语言应用](https://blog.csdn.net/lida2003/article/details/127828153)
2. [MarkDown教程](https://www.runoob.com/markdown/md-tutorial.html)

# 

工作方式：GitHub issue + tag的方式

## 任务标签

**鉴于项目涉及工种(硬件，软件，测试等)广泛，为更好的协调工作，形成阶段性成果，采用GitHub issue + tag的方式形成各种工作内容，详见下面分类：**

* Task：涉及SnapAirUnit工程代码的任务(比如：基线版本，编译构建和烧录指南等等)
* Feature：新增功能特性(比如：新增UDP forward x指令等等)
* Enhancement：功能优化(比如：算法从原来的O(n)变成O(1)等等)
* Bug：测试 & 使用过程发现的问题(比如：手机控制往前运动，结果飞机为按照期望运动等等)

## 任务描述

**为了快速理解任务，以下标签用于简单描述：**

* documentation：文档工作，建议大家采用md语法。
* duplicate：重复任务，请研发直接将重复的任务号批注后，assign给需求派发人员
* good first issue：--- bla..bla.. ---
* help wanted：需要帮助，请研发将需要帮助的内容批注后，assign给需求派发人员
* invalid：无效任务，请研发将无效原因批注后，assign给需求派发人员
* question：存在问题(需求不清，存在疑点或者设计冲突)，请研发分析问题后，批注意见，assign给需求派发人员
* wontfix：通常是经过需求派发人员协调讨论决策后，批注rootcause后标记

## Git代码提交

**为了更好的理解代码，比对分析代码，请每次commit提交代码注意以下工作：**

1. 使用的注释请用英文，不要使用中文字符。
2. 注释请言简意赅，并将在注释最前方将issue编号标注上"[#2]"。
3. 注释内容复杂，且不存在对应issue，请研发根据技术内容自建issue。

*注：工作细节决定后续调试分析的有条不紊，确保可分析、可追溯、可以找到rootcause，研发讲究的脚踏实地，循序渐进。*

# Project

## Coding rule
* 基线版本以后，新增代码所有注释请采用英文。 //预防某些编译器及IDE工具无法很好处理汉字
* 编码规范使用公司统一规范，如公司没有强制，请开发团队自建规范，并补充更新到文档中。  //建议考虑维持SDK基线版本编码风格

## VersionControl
[软件版本管理原则](https://blog.csdn.net/lida2003/article/details/36617839)
版本控制规范：x.y.commitid-clean/dirty/release
* x: 主版本号,由项目负责人维护
* y: 次版本号,由版本管理人维护
* commitid: 前7位，比如下面最初的版本，commit id = 39034f8
* clean/dirty/release: clean(与git版本一致), dirty(本地版本), release(发布版本)


> commit 39034f800063292df29c18e4002147890ce5a131 (HEAD -> main, origin/main, origin/HEAD)
> Author: aocodarc <105714332+aocodarc@users.noreply.github.com>
> Date:   Sat Nov 12 16:44:52 2022 +0800
> 
>     Initial commit

# Document

     .
    ├──> docs  //images used in *.md documents
    │   ├──> images
    │   ├──> Build_and_Flash.md
    │   ├──> How_to_Use.md
    │   └──> How_to_Connect.md
    └──> README.md

## 1. 编译构建固件方法

详见：[Build_and_Flash.md](./docs/Build_and_Flash.md)

## 2. 硬件如何连接飞控

详见：[How_to_Connect.md](./docs/How_to_Connect.md)

## 3. 如何使用SnapAirUnit

详见：[How_to_Use.md](./docs/How_to_Use.md)

