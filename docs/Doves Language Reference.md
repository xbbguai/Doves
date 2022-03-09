# Doves语言语法手册 Doves Language Reference

文档版本: 0.1a

Doves版本: 0.1a 初步测试版

Doves语言遵循开源协议GPLv3。


    Doves 
    V0.1a
	Copyrighted (C) 2022 Feng Dai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.



## 0. 概述
-------------------

### Doves语言简介

Doves语言是解释型的带有动态特征的计算机语言。Doves使用C++17写成，目前大约8,500行代码。可在macOS、Windows和Linux系统下编译。
Doves语言尚处于研发阶段，本文所描述的语法可能会有比较大的修改。

Doves语言的语法最基础的部分来自传统的Basic语言。但大多数特性和Basic语言已无多大关系。
Doves语言有两套关键字体系，一是英文的，另一套是中文的。两套关键字完全等价。
Doves语言不区分大小写。

Doves语言的源代码必须是utf8格式的文本文件。可以使用Visual Studio Code等工具书写。
Doves源代码可以带BOM信息也可以不带，但必须是utf8格式。
（尚未完成VSCode插件）

Doves语言的代码可以不以函数（过程）为组织单位，允许在源代码文件中直接书写代码逻辑。装载源代码文件时，Doves按照代码逻辑先后顺序解释执行。
Doves语言的源代码文件中如果有main()函数，装载源代码文件完成后，将执行main()函数。

Doves语言的关键字包括语句和内置函数名。内置函数名不能作为程序的其他标识符。
Doves语言的标识符为：以下划线（'_'）或者英文字母或者非ascii字符开始的、其他字符为下划线、英文字母、非ascii字符或者数字，长度不大于63字符串。超过部分将被截断并认为是同一个标识符。
无论是使用英文关键字系统还是中文关键字系统，Doves的内码都是unicode（Windows为UCS2，macOS/Linux为UCS4），标识符中可以带中文，可以使用中文为变量、对象等命名。

### 执行Doves语言程序

执行Doves语言源代码文件的命令为：

    doves [-c] 源代码文件名

其中 -c 开关用于指定源代码文件的关键字系统为中文。
如果需要向Doves程序传递参数，参数可写在源代码文件名后面。

### Doves源代码编译

Doves源代码中带有CMakeList.txt，在macOS/Linux之下可以使用cmake / make工具编译。
在Windows下编译时，请留意MSVC编译器不能读取不带BOM信息的源码。源码中的WordReader.h文件需要转成BOM开头的utf8文件。

## 1. 常量和变量
------------------------

### Doves的数据类型

Doves的数据类型有：

|英文关键字|中文关键字|描述|
|------|------|------|
|INTEGER|整数|等同于C/C++的long|
|REAL|实数|等同于C/C++的double|
|BOOLEAN|布尔|有TRUE、FALSE两种值|
|STRING|字符串|mac、linux内码为ucs4，windows内码为ucs2|
|ARRAY|数组||
|STRUCT/CLASS|结构/类||
|NULL|空|空类型是变量的默认类型。空类型变量第一次赋值时可赋予任何类型的值|

* 布尔类型和数值类型可互换互赋值。布尔true的值是1（请特别留意与Basic语言的不同，Basic是-1）、false是0；数值非0是true、0是false。

### Doves的常量

常量分为字面常量和常量两种。

字面常量的定义为：
* 数值型字面常量：在Integer类型可表达的范围内的字面量，将被解释为整型字面常量；否则，带有小数的、超过Integer能表示范围的、或者使用科学记数法表示的字面数值量，将被解释为实型常量
* 布尔型字面常量包括：true、false
* 字符串字面常量：用双引号括起来的内容为字符串字面常量

在程序中需要使用常量时，用CONST关键字声明。语法为：

英文

    CONST [GLOBAL] 常量名 = 常量值表达式

中文

    常量 [全局] 常量名 = 常量值表达式

常量值表达式中可以有变量。在常量声明时，一旦常量值计算出来之后，就不可再更改。
如果定义为GLOBAL，则该常量可在整个程序全局范围内使用。

示例

    CONST pi = 3.14


### Doves的变量

变量可以先声明后使用，也可以在需要的时候直接赋值使用。
变量的声明语句是DIM，语法如下：

英文

    DIM [GLOBAL] [BYREF] 变量名1 [,[BYREF] 变量名2 [, ...]] [AS 类型]

* DIM也用于定义数组、定义动态分配的内存空间。
* DIM可以同时定义多个变量、数组等，不同的变量用逗号隔开即可。
* GLOBAL指明本DIM语句将变量声明在全局区，整个程序的任何部分都可使用这个变量。如果不使用GLOBAL，则变量声明在函数内部，只在函数内部使用。
* BYREF指明所定义的变量是否是引用型变量。见“8. 引用和动态内存分配”。
* DIM语句定义的变量、数组等，定义之后将被赋予相关类型的自然初值。即：0（数值型）或者空（非数值型）。

示例

    定义三个变量：a, b, c。定义为Null类型（默认），然后将这三个变量分别赋予整数、字符串和布尔值：

    DIM a, b, c
    a = 10
    b = "string"
    c = true

注意：Doves语言可以书写不带函数的程序模块。比如以下程序可以直接执行：

    DIM a AS INTEGER
    INPUT a
    PRINT a * a

但这里的a却不是全局变量，而是在非函数区域范围内可使用的变量。比如要是想在上述程序之后的main()函数中使用a将报错：

    FN main()
        PRINT a //这条语句将报错，提示标识符a未定义
    ENDFN

如果希望所有程序的其他部分可以使用a，需要将a声明成全局变量：

    DIM GLOBAL a AS INTEGER

如果不指定类型，则变量的默认类型为NULL。一个NULL类型的变量，第一次赋值时可以赋予任何类型的值。

变量不能随意更改自己的类型。对于NULL型变量，一旦经过赋值，类型就确定，之后就不能更改。
变量赋值时，会进行类型匹配。数值型变量可以赋予数值。Doves不会对不同数值类型之间转换造成的精度损失给出警告，请在编写程序时务必留意。
字符串变量只能赋予字符串值。

DIM语句也用来定义数组。见“7. 数组”。

在赋值语句中，变量可以不声明直接使用。比如：

    a = 5 * 10

如果a之前未定义，Doves将先自己定义一个NULL类型的变量a，而后计算等号后面的表达式，再根据表达式的结果类型，确定a的类型并赋值。

### Doves的字符串

Doves的字符串独立的基本数据类型，可以对字符串变量赋值，可以对字符串相加做连接运算。
Doves提供一组字符串操作函数，包括：
|英文定义|中文定义|描述|
|-------|-------|----|
|ASC(str AS STRING) AS INTEGER||取str第一个字符的内码|
|CHR(ch AS INTEGER) AS STRING||返回内码为ch的字符串|
|VAL(str AS STRING) AS REAL||将str的内容转换成数值|
|LEN(str AS STRING) AS INTEGER||返回字符串长度|
|TOUPPER(str AS STRING) AS STRING||将字符串转换为大写|
|TOLOWER(str AS STRING) AS STRING||将字符串转换为小写|
|LEFT(str AS STRING, length AS INTEGER) AS STRING||取字符串左边length个字符|
|RIGHT(str AS STRING, length AS INTEGER) AS STRING||取字符串右边length个字符|
|MID(str AS STRING, start AS INTEGER, length AS INTEGER) AS STRING||从字符串的第start个字符起，取length个字符|
|FIND(str AS STRING, sub AS STRING, from AS INTEGER) AS INTEGER||从字符串中的第from个位置起，找sub字串所在位置。如果找不到，返回-1|
|REPLACE()||尚未实现|


Doves的内置函数可以使用标准的函数调用语法调用，也可使用“.”运算符调用。方法是在操作数后加上“.函数名(参数表)”，函数参数表省略第一个函数参数，Doves将操作数自动当成第一个参数。
  
比如：

    s = "string"    
    PRINT LEN(s)    //求s的长度
    PRINT s.LEN()   //与上个语句效果相同


## 2. 表达式
------------------------

Doves的表达式和Basic语言一致。算术运算符号有：
* +, -, *, /, \ (整除), MOD (模运算), ( 和 ), ^ (幂运算)
  
条件运算符有：
* =, >, >=, <, <=, <>, AND, OR, NOT, IS, IS NOT

逻辑运算符有：
* AND, OR, NOT, XOR

其中：AND、OR、NOT用在条件表达式上时，作为条件运算符。比如 a > b AND b > c 表示a > b和b > c两个条件均满足时为true。
如果用在整数上，则进行逻辑运算。比如：255 AND 127的值为127。

运算符优先级为：
()、+/-(正号负号)、^、* /、\、MOD、+/-、=/>/>=/</<=/<>/IS/IS NOT、NOT、AND、OR、XOR

IS/IS NOT运算符：
IS/IS NOT运算符用于判断一个变量的类型。语法为：

    表达式 IS 类型
    表达式 IS NOT 类型

比如: a IS INTEGER，如果a是整数，则表达式的值是TRUE，否则是FALSE。
请注意：IS NOT是一个完整的运算符，NOT则是另一个运算符。当然， a IS NOT INTEGER 和 NOT (a IS INTEGER) 结果一样。

需要留意：
* Doves的表达式是Basic语言语法，与C一族的语言区别明显：没有 +=、++ 这一类运算符、相等比较为单等号而不是双等号、逻辑与或非和条件与或非运算符相同等等。
* Doves 0.1a 没有实现条件表达式短路。所有的分支都会被计算。所以要留意程序中条件表达式的书写。

## 3. 输入和输出
------------------------

### PRINT语句

PRINT语句遵循传统Basic语言的语法。
PRINT语句用来在字符界面打印输出任意文本内容。
PRINT语句后可以跟任何表达式。Doves输出表达式的值。
PRINT语句可以按格式输出。每个输出项如果以","结尾，则按终端的tab设置隔开每一个输出项；如果以";"结尾，则两个输出项前后连接在一起。
比如：

    PRINT "The result is"; 5 + 10, 10 + 20, 20 + 30

Doves提供Tab、SetW和SetPrecision三个函数，来设置PRINT的输出格式。
* Tab(n)：打印n个空格(Windows) / 将当前光标移动到本行第n个位置(Linux/macOS)
* SetW(n): 下一个打印项的宽度为n个字符
* SetPrecision(n)：下一个浮点数打印时，小数点的精度为n格字符
  
比如：

    PRINT SetW(5); "123"; SetPrecision(5); SetW(10); 123.45

### INPUT语句

INPUT语句遵循传统Basic语言的语法。
INPUT用于从键盘输入值。INPUT的语法是：

    INPUT ["提示信息",] 变量1 [AS 类型1] [, 变量2 [AS 类型2] [, ...]] 

INPUT从键盘的输入内容读入数据，并以类型1赋值给变量1；然后读入数据，以类型2赋值给变量2，...
如果不指定类型，则按变量声明时的类型作为输入类型。

注意：
* 如果变量未经过声明就在INPUT语句中使用，而且没有指定输入的类型，则INPUT按字符串处理输入的内容。
* 如果变量经过声明，并且声明了具体的类型，则INPUT按变量类型来处理输入的内容。
* 如果变量经过声明，并且声明了具体类型，但INPUT语句中指定的输入类型与之不匹配，系统将报错。

INPUT还可以显示一段提示信息。


## 4. 分支
------------------------

### IF 语句

    IF 条件表达式1 [THEN]
        语句块1
    [ELSE IF 条件表达式2] [THEN]
        语句块2
    ...
    [ELSE]
        语句块n
    ENDIF

在Doves语言中，IF后面的THEN可以省略不写。

示例

    IF a > 10 AND b < 10
        PRINT a
    ELSE
        PRINT b
    ENDIF

### SELECT...CASE 语句

    SELECT [CASE] CASE表达式
        CASE 值范围表达式1
            语句块1
        CASE 值值范围表达式2
            语句块2
        ...
        CASE ELSE
            语句块n
    ENDSELECT

其中：
* CASE表达式是任意能计算出一个值的表达式，比如 a * 100
* 值范围表达式满足条件，则进入相应CASE的语句块执行。值范围表达式的形式有如下几种：
  * 单个数值 (表示CASE表达式的值与此数值相等)
  * 单个表达式的值 (表示ASE表达式的值与此表达式的值相等)
  * 表达式1 to 表达式2 (表示CASE表达式的值落入[表达式1, 表达式2]值范围内)
  * 带IS的条件表达式，比如：IS > 100 (表示ASE表达式的值 > 100)
  * 以上任意条件的组合，用逗号隔开。表示用逗号隔开的任意条件之一成立即成立

示例

    SELECT a + b
        CASE 0
            PRINT "Nothing changed."
        CASE 1 TO 100, 200
            PRINT "a+b 在1到100之间或者等于200"
        CASE IS > 300
            PRINT "a+b > 300"
        CASE ELSE
            PRINT "其他情况"
    ENDSELECT

## 5. 循环
------------------------

### FOR 语句

    FOR 循环控制变量 = 初始值表达式 TO 结束值表达式 [STEP 步长值表达式]
        循环体
    NEXT | ENDFOR

注意：循环结束语句可以是传统Basic语法所用的NEXT，也可以使用ENDFOR。如果使用NEXT，注意NEXT后面不可加循环控制变量。

示例

    FOR i = 1 TO 100 STEP 2
        sum = sum + i
    NEXT

### WHILE 语句

    WHILE 条件表达式
        循环体
    WEND | ENDWHILE

注意：循环结束语句可以是传统Basic语法所用的WEND，也可以使用ENDWHILE。

### LOOP 语句

    LOOP
        循环体
    ENDLOOP 条件表达式

### FOREACH 语句

    FOREACH 变量 IN 数组
        循环体
    NEXT | ENDFOR

注意：FOREACH语句中的变量，无论一开始是什么类型，将在执行过程中自动改为BYREF，以便引用数组中的元素。数组中的每个元素类型可能不一样，所以这个变量每次所引用的数据类型也会不一样。FOREACH循环结束后，这个变量将指向数组中的最后一个元素。

在使用FOREACH时务必要注意上述细节。

### BREAK 和 BREAK ALL

BREAK 语句用于在循环中退出（注意：Basic语言的BREAK是中断程序执行。Doves用PAUSE语句来实现程序中途暂停，以便调试）。
如果在多重循环中，可以使用BREAK ALL从内层循环直接退到最外层循环之外。

以下示例中，当i * j = 100时，将跳出所有循环：

    FOR i = 1 TO 20
        FOR j = 1 TO 20
            IF i * j = 100
                BREAK ALL
            ENDIF
        NEXT
    NEXT
    PRINT i, j


## 6. 函数
------------------------

### 函数体

    FN 函数名([形式参数表]) [AS 返回值类型]
        函数体
        [RETURN [返回值表达式]]
    ENDFN

如果函数体不规定返回值类型，则函数可以返回任意类型的值，也可不返回任何值。

### 函数的形式参数表

    [BYREF] 参数名1 [AS 参数类型] [, [BYREF] 参数名2 [AS 参数类型]] ...

其中：
* BYREF 表示该参数为引用参数
* 如果不规定参数类型，则可以传入任何类型的参数值

### 函数调用

    函数名([实际参数表])

* 函数可在任何表达式中调用
* 函数可在语句中直接调用
* 函数的返回值可赋予另一个变量
* 函数需要先定义，后调用。也就是说函数的定义代码需要写在函数被调用之前。

### 函数的实际参数表

    参数值表达式1 [AS 参数名n1], [参数值表达式2 [AS 参数名n2]] ...

* 调用函数时，在函数的形式参数表中定义的参数，均需要提供
* 如果不指定实际参数对应的参数名，则按顺序进行形实转换
* 请不要在实际参数表中混用带参数名指定/不带参数名指定的形式。如果混用，则不给出参数名的实际参数，按该参数的顺序进行匹配。


## 7. 数组
------------------------

### 数组的定义

Doves使用DIM语句来声明数组：

    DIM [GLOBAL] [BYREF] 数组名1[第0维长度表达式][[第1维长度表达式][...[第n维长度表达式]]] [, 数组名2[第0维长度表达式][[...]] ... ] [AS 类型]

也可以直接通过赋初始值来定义数组，无需使用DIM语句。格式为：

    数组名 = [数组元素值0, 数组元素值1, ...]

其中，数组元素本身也可以是数组。数组元素值可以是计算的结果（包括变量）。

* 数组需要先定义后使用
* GLOBAL指定数组的保存位置。如果指定了GLOBAL，则数组保存在全局空间，所有其他函数和模块都可使用该数组。否则，数组保存在函数空间，只能在函数内使用
* 长度表达式表示数组的大小，不可以小于等于0
* 如果不指定类型，则数组中的任意一个元素，可以存放任意一种类型的数据，包括数组本身
* 如果指定数组为 BYREF，则数组中的每个元素都是引用类型，需各自指定其所引用的变量后使用

示例1

    定义一个大小为n * n的二维数组
    DIM n AS INTEGER    //确保下面的输入语句输入的内容转换为整数
    INPUT n
    DIM arr[n][n]

示例2

    直接定义一个带数据的数组变量
    arr1 = [1, 5 + 10, 20, 33]
    arr2 = [3, 4, 5, [1, 2, 3], "test", [true, true, "fine"]]

### 数组的使用

#### 数组元素的引用

数组元素通过下标来引用。下标的范围为 0 到 长度表达式 - 1。格式如下：

    数组名[第0维下标][[第1维下标][...[第n维下标]]]    (注：下标的方括号不是可选符号，而是下标符号)

比如，对于二维数组arr，可以这样访问其元素：

    arr[x][y]

而对于这样一个数组：

    arr2 = [3, 4, 5, [1, 2, 3], "test", [true, true, "fine"]]

不难发现：

    arr2[0] 的值是 3
    arr2[1] 的值是 4
    arr2[2] 的值是 5
    arr2[4] 的值是 "test"

但是：arr2[3]本身是个数组。可以这样访问arr2[3]中的元素：

    arr2[3][0] 的值是= 1
    arr2[3][1] 的值是 2
    arr2[3][2] 的值是 3

#### 数组赋值

对于两个数组变量，可以互相赋值。比如 arr1和arr2：

    DIM arr2[10]
    arr1 = [1, 2, 3, 4, 5]
    arr2 = arr1

则：arr1中的元素将一个个按顺序拷贝到arr2中。此时有如下规则：
* 对于被赋值的数组，如果事先定义，其长度不能小于提供数据的数组。
* 对于被赋值的数组，在进行数组元素拷贝时，各个元素的数据类型必须兼容。

#### 临时数组（元组）

Doves没有元组的专门语法，但是可以使用匿名的临时数组来实现元组的功能。
临时数组的语法是：

    [数组元素0 [, 数组元素1, ...]]

当临时数组作为左值时，所有元素都必须是可引用型（变量）。当临时数组作为右值时，元素可以是计算值。

示例

    以下语句可实现a, b数据交换：

    a = 10
    b = 20
    [a, b] = [b, a]

### Doves数组的处理方式

* Doves的数组，如果类型定义为Null（或者不定义类型），则每个元素是可以储存任意类型的数据的
* Doves的数组元素，可以定义为BYREF。这样每个元素可以引用其他数据
* Doves的多维数组，实际上是：第n维的每个元素，各自引用一个第n+1维的数组。比如：
    
    DIM ARR[5][10] AS INTEGER
    
    对于这个二维数组，在执行DIM语句时，系统先这样定义：
    
        DIM BYREF ARR[5]
    
    然后执行一段类似这样的代码：

        FOR i = 0 to 4
            DIM @(ARR[i])[10] AS INTEGER //将变量ARR[i]定义为指向一个长度为10的、类型为INTEGER的一维数组
        NEXT
    
    关于@的语法和DIM，见后面的“8. 引用和动态内存分配”一节中详细描述。

由于Doves的多维数组，每一维的每个元素都是下一维的数组，所以每一维都可以单独访问。


### 数组大小重定义

    REDIM [(]数组[)][新长度表达式]

REDIM语句可以重新定义数组的长度。注意如果新长度比原来短，则数组内容会被切断。
REDIM语句只能定义一个维度的长度。
由于多维数组在Doves中的处理方式就是“数组的数组”，所以我们可以拆出其中一维的任意一个元素来重新定义其大小。比如：

    DIM ARR[5][5] AS INTEGER

    我们可以这样REDIM:

    REDIM ARR[4]

    则数组ARR就变成了： ARR[4][5]。注意数组内的数据会被截断。

    我们还可以这样REDIM：

    REDIM (ARR[0])[100]

    这个时候，ARR[0]原先所指向的5个元素的一维数组，就会变成100个元素。此时，ARR[0]到ARR[4]各自的元素个数就是：
        ARR[0]: 100个。可访问的范围：ARR[0][0] ~ ARR[0][99]
        ARR[1]: 5个。可访问的范围：ARR[1][0] ~ ARR[1][4]
        ARR[2]: 5个。可访问的范围：ARR[2][0] ~ ARR[2][4]
        ARR[3]: 5个。可访问的范围：ARR[3][0] ~ ARR[3][4]
        ARR[4]: 5个。可访问的范围：ARR[4][0] ~ ARR[4][4]

### 数组与函数

如果需要传递数组给函数，函数的参数应定义为ARRAY类型，或者干脆不指定这个参数的类型。
数组传递给函数时，将会被完整复制。通常应该避免将大数组用这种方法传递给函数。Doves提供了引用类型，可将函数参数定义为BYREF，也就是该参数是引用类型，而后传递数组时，就不会复制一遍数组，而是在函数内引用该数组。

函数可以返回一个数组。返回的数组会完整复制给接收函数返回值的变量。


## 8. 引用和动态内存分配
------------------------

### 定义引用型变量

在使用DIM定义变量时，在变量名前加上BYREF，即可将该变量定义为引用型变量：

    DIM BYREF 变量名1 [AS 类型]

引用型变量不能直接使用，需要指定其所引用的变量之后，才可使用。引用型变量可以理解为被引用变量（目标变量）的别名，它们在使用上完全等价，只是名称不同。
不像其他数值变量那样可以不定义直接使用，引用型变量需要先定义。
定义引用型变量时，如果不指定其类型，则该引用型变量可以引用任意类型的变量。

### 指定引用

    引用型变量 @= 变量

如此，就完成了该引用型变量对另一个变量的引用。可以引用另一个引用型变量，此时，这两个引用型变量都引用同一个目标变量。比如：

    a = 100
    DIM BYREF aref1
    DIM BYREF aref2
    aref1 @= a
    aref2 @= aref1

这样，aref1和aref2同时引用a。

注意：Doves语言的引用型变量可以多次更改引用目标。

如果需要去除对变量的引用，可用如下语法：

    引用型变量 @= NULL

### 引用和类型

* 如果定义了引用型变量的类型，则该变量只能引用相同类型的目标变量
* 如果不指定引用型变量的类型，则该变量第一次引用目标变量时，可引用任意类型的变量。此后，只能引用与此相同类型的变量

### 动态内存分配

    DIM [GLOBAL] @(引用型变量名) AS 类型

引用型变量可用于指向动态分配的内存。动态分配的内存通过引用计数自动管理。
首先需要定义一个引用型变量，然后可以动态分配内存，通过引用来使用此内存。比如：

    DIM BYREF mem AS INTEGER
    DIM @(mem) AS INTEGER

注意：定义mem时也可不指定类型。
这样，mem将指向一个新分配的、INTEGER类型的内存区域。该内存区域通过引用计数进行管理。当引用计数为0时，自动在下一轮垃圾回收过程里回收。

引用型变量可引用一个动态分配的数组：

    DIM @(引用型变量名)[长度] AS 类型

比如：

    DIM BYREF arr AS ARRAY
    DIM @(arr)[100] AS INTEGER

而后，arr就是一个长度为100的整型数组了。
注意这里定义arr的时候可以不指定其为ARRAY类型。

我们还可以使用REDIM来重新分配一片内存区域：

    REDIM @(arr)[50]

注意：不能在动态内存分配中使用多维数组语法。动态内存分配时，只能使用一个维度。


### 函数与引用

函数参数可以定义为引用参数。如果是引用参数，则在函数内部对该参数的任何写入操作，都会影响函数外部所穿入的变量的值。
函数不能将函数内定义的变量作为引用目标。因为这样的变量在函数退出后就会被销毁。


## 9. 结构与类
------------------------

### 结构体的定义

Doves语言的类与结构体完全等价。定义一个类/结构体的语法为：

    STRUCT | CLASS 结构体名 [EXTENDS 父结构体名]
        [DIM] [BYREF] 成员变量名 ｜ 成员数组名[数组长度] [AS 类型]
        ...
    ENDSTRUCT | ENDCLASS

由于STRUCT和CLASS完全等价，加上Doves默认一切都是值类型，所以建议只使用STRUCT。
结构体没有构造函数，也没有析构函数。如果需要，请自己定义并显式调用。声明一个结构体变量时，系统初始化每个成员变量为0或者与0相似的其他类型（False、空字符串等）。
结构体需要先定义，而后才能使用结构体声明变量。结构体变量也就是类的对象。
在定义结构体中的成员变量时，DIM关键字可以省略。
结构体中可以定义数组，也可以定义引用类型的成员变量。
如果结构体中有引用类型变量，这个变量在结构体变量刚声明时，是不可用的，需要将其引用到具体变量后，才可以使用。
结构体中的引用型成员，可以引用另一个本结构体变量，以便构造链表、树等动态结构。

成员变量和一般变量一样，可以在一开始的时候不指定任何类型。一旦初次赋值，该变量的类型就会确定下来。

### 结构体成员函数

结构体成员函数写在结构体外部。语法为：

    FN 结构体名.成员函数(形式参数表) [AS 返回值类型]
        函数体
        [RETURN [返回值]]
    ENDFN

结构体函数语法和普通函数一致。不同的只是该函数属于某个结构体，只有该结构体的对象可以使用。
结构体函数需要先定义，后使用。
函数体中需要使用结构体成员时，可在成员名之前加"self."。比如：
    
    STRUCT Coordinate
        x AS INTEGER
        y AS INTEGER
    ENDSTRUCT

    FN Coordinate.Move(cx, cy)
        self.x = self.x + cx
        self.y = self.y + cy
    ENDFN

    FN Coordinate.MoveLeft(cx)
        self.Move(cx, 0)
    ENDFN

self可省略。但如果函数参数表中带有与成员变量相同的名称时，需要使用self来区别出成员变量。
建议在写程序时尽量使用self，因为有了self限定查询范围，执行速度更快。

### 结构体继承

定义结构体时，使用EXTENDS关键字定义继承关系。如果一个结构体从父结构体继承，那么它将拥有父结构体所有的成员变量和成员函数。
注意：Doves不限制公共、私有、保护等成员属性。

父结构体的变量（对象）不能使用任何子结构体内的成员。

如果结构体存在继承关系，子结构体不能有和父结构体相同的成员变量名，但可以有同名的成员函数。子结构体的同名成员函数将覆盖父结构体的成员函数。

### 结构体数组

可以声明结构体数组。声明结构体数组和声明普通数组一样，只是类型使用定义好的结构体名。比如：

    STRUCT Coordinate
        X AS INTEGER
        Y AS INTEGER
    ENDSTRUCT

    DIM Coors[100] AS Coordinate
    FOR i = 0 TO 99
        Coors[i].X = X + 1
        Coors[i].Y = X * X
    NEXT

### 结构体成员的使用

声明了结构体变量（对象）后，可以使用“.”运算符来指向结构体中的成员。包括成员变量和成员函数。比如：

    STRUCT Coordinate
        X AS INTEGER
        Y AS INTEGER
    ENDSTRUCT

    FN Coordinate.Draw()
        PRINT X,Y
    ENDFN

    DIM Coor AS Coordinate
    Coor.X = 10
    Coor.Y = 20
    Coor.Draw()

### 结构体赋值

类型相同的两个结构体变量可以使用“=”做赋值操作，将一个结构体变量中所有成员的数据复制到另一个结构体变量中。
比如：

    DIM Coor1 AS Coordinate
    DIM Coor2 AS Coordinate
    ...
    Coor1 = Coor2   //Coor2每个数据字段的值，都会复制到Coor1中。

另外，可以使用临时数组给结构体赋值。Doves按结构体成员变量顺序和数组元素顺序复制数据。比如上述的Coordinate结构体：

    DIM Coor AS Coordinate
    Coor = [100, 200]

则：Coor.x = 100，Coor.y = 200。

注意：
* 数组中的元素类型需要与结构体中成员变量类型一致。
* 数组中的元素个数不能大于结构体成员变量个数。

### 结构体与函数

结构体变量（对象）可以作为函数参数传递给函数。传递时，将复制整个对象数据。
可以将结构体变量以引用形式传递给函数。这样，在传递时将不产生对象内容复制，但函数体内对该对象的任何修改，都将反应到函数外部。

如果函数返回结构体，返回时将产生对象复制。


## 10. 多态
------------------------

Doves通过引用来实现多态。
定义一个结构体变量为引用类型，可用这个变量来引用该结构体的变量或者该结构体的子类变量。比如：

    STRUCT Father
        a   //这种没有指定任何类型的变量为Null型，可以初始化为任何数据类型
        b
        c
    ENDSTRUCT

    FN Father.SayMyself()
        PRINT "I'm the father object"
    ENDFN

    STRUCT Son EXTENDS Father
        d
    ENDSTRUCT

    FN Son.SayMyself()
        PRINT "I'm the son object, I've got d. d = "; self.d
    ENDFN

    DIM BYREF ref AS Father

    DIM objFather AS Father
    DIM objSon AS Son

    ref @= objFather
    ref.a = 10
    ref.SayMyself() //将调用Father.SayMyself()

    ref @= objSon
    ref.d = 20
    ref.SayMyself() //将调用Son.SayMyself()

同样地，可以用一个父类的引用型变量，来动态定义出对象：

    DIM BYREF dynaObj AS Father
    DIM @(dynaObj) AS Son
    dynaObj.d = 50

声明时不指定具体类型的BYREF变量，首次使用时，可引用任何结构体变量。一旦引用过某种结构体变量，则之后只能引用该结构体或者该结构体的子结构体变量。


## 11. 结构化异常处理
------------------------

    TRY
        [THROW [表达式]]
    CATCH [变量1 [AS 类型]]
        异常处理代码
        [RETRY]
    CATCH [变量2 [AS 类型]]
        异常处理代码
        [RETRY]
    ENDTRY

每个CATCH块，可接收不同类型的异常抛出。CATCH通过定义接收变量的类型，来区别要接收什么样的异常。
TRY块中可用THROW来抛出异常。THROW后所跟的表达式的值的类型，关系到哪个CATCH块可以接收到该异常。
TRY块中如果出现系统级异常，Doves将抛出Exception类的对象。Exception类是Doves内置的类，其定义为：

    STRUCT Exception
        errCode AS INTEGER
        line AS INTEGER
        column AS INTEGER
        moduleIdx AS INTEGER
    ENDSTRUCT

errCode:    错误代码
line:       错误所在行
column:     错误所在列
moduleIdx:  错误所在模块编号

如果存在接收变量类型为Exception的CATCH块，则该CATCH块可以接收到系统级异常的错误信息。

如果CATCH块不带任何接收变量，则所有未被其他CATCH块接收的异常，都将被此CATCH块接收。

可以RETRY是Doves语言的特点之一。在CATCH块中的RETRY语句可以让程序返回TRY块重新执行。这样，当错误产生时，可以用CATCH来处理，而后可以用RETRY来重试。


## 12. 二进制内存区域
------------------------

Doves被设计为没有指针、无需程序员管理内存的语言。但有时候我们需要处理二进制数据，尤其是在读写文件、调用其他语言编写的模块时。为此，Doves内置二进制存储区结构体Binary，用于这类需求。

Binary结构体的定义如下：

    STRUCT Binary
    ENDSTRUCT

Binary结构体没有对外提供任何可以访问的成员变量，所有操作通过成员函数进行。
Binary的成员函数如下：

    Create(size AS INTEGER)
    使用Binary的对象之前，需要调用Create来分配存储区大小。参数size是以字节为单位的存储区大小值。

    SetAlign(alignsize AS INTEGER)
    设置该存储区对数据的对齐长度。对齐长度是指：所有存入存储区的数据，必须以对齐长度的整数倍占用存储空间。默认对齐长度为1，也就是1字节。在调用C语言库函数时如果需要交换结构体数据，需要注意对齐问题。

    GetSize() AS INTEGER
    返回存储区大小值。

    GetAt(index AS INTEGER) AS INTEGER
    返回存储区内位置为index的字节的值。

    SetAt(index AS INTEGER, byte AS INTEGER) AS BOOLEAN
    将存储区内位置为index的字节的值设置为byte。byte的范围只能时0～255。成功返回TRUE，如果越界或出错抛出异常。

    PutInteger(index AS INTEGER, data AS INTEGER, type AS STRING) AS INTEGER
    将存储区内index字节的位置为开始位置，写入一个整数值data。type规定了该整数值的格式。type是字符串，取值范围为：
    * "long"
    * "int"
    * "short"
    * "char"
    PutInteger按对齐方式放置数据，最后返回的是，以index为开始位置放入数据后，下一个可放入数据的位置。用户可以用这个返回值作为下一个需要写入的位置的起始位置值。
    
    PutReal(index AS INTEGER, data AS REAL, type AS STRING) AS INTEGER
    将存储区内index字节的位置为开始位置，写入一个浮点数值data。type规定了该整数值的格式。type是字符串，取值范围为：
    * "float"
    * "double"
    PutReal按对齐方式放置数据，最后返回的是，以index为开始位置放入数据后，下一个可放入数据的位置。用户可以用这个返回值作为下一个需要写入的位置的起始位置值。

    PutString(index AS INTEGER, data AS STRING, encoding AS STRING) AS INTEGER
    将存储区内index字节的位置为开始位置，写入一个字符串data。encoding规定了对于该字符串，在存储区中如何编码。encoding是字符串，取值范围为：
    * "utf8"
    * "ucs2"
    * "ucs4"
    PutString按对齐方式放置数据，最后返回的是，以index为开始位置放入数据后，下一个可放入数据的位置。用户可以用这个返回值作为下一个需要写入的位置的起始位置值。

    GetInteger(index AS INTEGER, BYREF data AS INTEGER, type AS STRING) AS INTEGER
    从存储区的index字节的位置为开始位置，读出一个整数写入data。type规定了该整数值的格式。type是字符串，取值范围为：
    * "long"
    * "int"
    * "short"
    * "char"
    GetInteger按对齐方式取数据，最后返回的是，以index为开始位置取出数据后，下一个取数据的位置。用户可以用这个返回值作为下一个需要读取的位置的起始位置值。

    GetReal(index AS INTEGER, BYREF data AS REAL, type AS STRING) AS INTEGER
    从存储区的index字节的位置为开始位置，读出一个浮点数写入data。type规定了该浮点数的格式。type是字符串，取值范围为：
    * "float"
    * "double"
    GetReal按对齐方式取数据，最后返回的是，以index为开始位置取出数据后，下一个取数据的位置。用户可以用这个返回值作为下一个需要读取的位置的起始位置值。

    GetString(index AS INTEGER, BYREF data AS STRING, encoding AS STRING) AS INTEGER
    从存储区的index字节的位置为开始位置，读出一个字符串写入data。encoding规定了该字符串在存储区中的编码格式。encoding是字符串，取值范围为：
    * "utf8"
    * "ucs2"
    * "ucs4"
    GetString按对齐方式取数据，最后返回的是，以index为开始位置取出数据后，下一个取数据的位置。用户可以用这个返回值作为下一个需要读取的位置的起始位置值。


## 13. 时间
------------------------

Doves内置一个时间结构。定义如下：

    STRUCT DateTime
        year AS INTEGER
        month AS INTEGER
        day AS INTEGER
        hour AS INTEGER
        minute AS INTEGER
        second AS INTEGER
    ENDSTRUCT

DateTime有一个成员函数用于获取当前时间并填入各个成员函数:
    GetNow()


## 14. 文件
------------------------

Doves语言内置File结构，用于处理文件的读写。
File结构体的定义如下：

    STRUCT File
        name AS STRING
    ENDSTRUCT

File结构体有如下成员函数：

    Open(write AS BOOLEAN, binary AS BOOLEAN, plus AS BOOLEAN) AS BOOLEAN
    打开文件。打开文件之前，需要设置name成员为文件的路径名。打开成功返回true，否则返回false。
    参数write：= true 写 / = false 读。
    参数binary： = true 以二进制方式读写 / = false 以文本方式读写
    参数plus：= true 可读可写 / = false 只读只写

    Close() AS BOOLEAN
    关闭文件。成功返回true，失败返回false。

    Read(BYREF readTo [, ...]) AS BOOLEAN
    以文本格式读入数据并保存到readTo中。
    读入的数据是什么格式，以readTo的类型决定。如果是BOOLEAN、INTEGER或REAL类型，读入数值。如果是STRING类型，读入字符串。
    Read函数读入的内容，以空格或者换行符为间隔。
    Read可以有多个readTo参数。如果有不止一个，则按每个readTo参数的类型读入数据并分别保存。注意参数个数最大为99。
    注意：
    * 目前的Doves，以文本格式读入文件时，只支持utf8编码。
    * 读入字符串时，最大字符串长度为1023。大于这个长度的字符串，可分多次读入。

    ReadLn(BYREF readTo AS STRING [, ...]) AS BOOLEAN
    以文本格式读入一行内容并保存到readTo中。可以有多个readTo参数。如果不止一个，则读入多行数据，每行按顺序保存到各自的readTo参数中。注意参数个数最大为99。
    注意：
    * 目前的Doves，以文本格式读入文件时，只支持utf8编码。

    GetChar() AS INTEGER
    读入一个字符（数值范围0～255）并返回。

    Write(toWrite [, ...])
    以文本格式写入数据。
    写入的数据是什么，以toWrite类型决定。如果写入字符串，将编码为utf8。写完数据后会写入一个空格作为间隔。
    Write可以有多个toWrite参数。如果有不止一个，则按参数顺序写入，并以空格隔开。

    WriteLn(toWrite [, ...])
    以文本格式写入一行数据。
    写入的数据是什么，以toWrite类型决定。如果写入字符串，将编码为utf8。写完数据内容后会写入换行符，表示一行结束。
    WriteLn可以有多个toWrite参数。如果有不止一个，则按参数顺序写入，并以换行符隔开。

    Seek(pos AS INTEGER, from AS INTEGER) AS BOOLEAN
    改变文件当前读写位置。pos是相对位移量，以字节为单位；from是相对位置点，取值范围是：
    * 0: 从文件开头起
    * 1: 从当前位置起
    * 2: 从文件末尾起

    Tell() AS INTEGER
    返回文件当前读写位置。

    ReadBin(BYREF readTo AS Binary, index AS INTEGER, size AS INTEGER) AS BOOLEAN
    以二进制方式读入数据，存放到readTo中。readTo是Binary结构体变量，并且已初始化。
    参数index为readTo的开始写入位置，size为要从文件中读入多少字节。

    WriteBin(BYREF toWrite AS Binary, index AS INTEGER, size AS INTEGER) AS BOOLEAN
    以二进制方式写入数据。toWrite是Binary结构体变量，并且已存入数据。
    参数index为toWrite的开始位置，size为要写入多少个字节。


## 15. 线程
------------------------

Doves语言内置Thread结构和Mutex结构，用于实现多线程。

### Thread结构

Thread结构的定义如下：

    STRUCT Thread
    ENDSTRUCT

Thread内置两个成员函数：

    IsRunning() AS BOOLEAN
    如果线程处于运行状态，返回true。如果线程尚未运行或已经退出，返回false。

    Start()
    启动线程运行。

要实现一个线程，首先需要从Thread结构体派生一个结构体，比如：

    STRUCT MyThread EXTENDS Thread
    ENDSTRUCT

而后，实现一个名为Runner的成员函数，这个函数是线程的工作函数，线程启动后，在后台运行这个函数。

    FN MyThread.Runner()
        PRINT "Thread is running."
        FOR i = 0 TO 9
            PRINT i
            Sleep(100)
        NEXT
        PRINT "Thread is ending."
    ENDFN

在定义一个MyThread结构体变量后，就可以用Start()函数来启动线程运行：

    DIM thd AS MyThread
    ...
    thd.Start()
    PRINT "Thread started."

以上代码，将在MyThread.Runner()函数运行的同时，打印"Thread started."。

### 线程的运行和退出

Doves极度简化了线程。一旦线程启动，就需要等待线程运行完成。
线程的运行如果需要和其他线程交互数据，可通过在定义线程结构时加入成员变量来进行。比如可在成员变量中加入一个BOOLEAN，在线程的Runner中用这个值来控制退出循环结束线程。

这里有个问题，就是线程对象的声明位置。比如上述的MyThread结构，如果在全局区域声明一个MyThread对象，则：这个线程将在全局范围内运行，并且，当程序主线程结束后，整个程序需要等待所有全局线程运行完毕退出后，程序才退出。

如果线程对象声明在函数空间内，比如：

    FN MyFunction()
        DIM thd AS MyThread
        thd.Start()
        ...
    ENDFN

此时，如果调用MyFunction()，thd线程将会被启动并且在后台运行，MyFunction函数在执行完thd.Start()之后将继续执行其他代码。当MyFunction函数执行完成后，需要判断thd线程的情况。如果thd线程已经执行完毕，则函数退出、返回。否则，函数需要等待thd执行完成，才会返回。

### Mutex结构体

Doves内置Mutex结构体，用于线程同步。Mutex定义如下：

    STRUCT Mutex
    ENDSTRUCT

Mutex有两个成员函数：

    Lock()
    关锁。

    Unlock()
    开锁。

对于需要同步的线程，使用同一个Mutex对象，调用Lock()来获得信号量。完成后用Unlock()释放信号量。


## 16. 模块
------------------------

### 导入其他Doves语言写的模块

Doves语言可以导入其他Doves语言程序模块。语法为：

    IMPORT "文件路径名" [AS 模块名]

如果不指定模块名，Doves以文件名作为模块名。
在使用其他模块中的函数、结构体、对象、变量时，要使用：

    模块名.对象名

这样的格式。

如果导入的模块中存在函数外代码，则所有函数外代码在导入时会被执行一遍。
如果导入的模块中存在main函数，该main函数不会被执行，但可以在主模块中通过 模块名.main() 来调用。

### 导入C语言动态链接库（尚未实现）

Doves语言可以导入C语言写的动态链接库。语法为：

    IMPORT LIB "文件路径名" [AS 模块名]


## 17. 事件驱动引擎（尚未实现）

### POOL结构体

### WHEN语句

### ALWAYS语句

### WIRE语句

### MESSAGE结构体

### POSTMESSAGE语句

### ONMESSAGE语句

------------------------

## 18. 二维绘图（尚未实现）
------------------------

## 19. 游戏引擎（尚未实现）
------------------------

## 20. 基础数据结构（尚未实现）
------------------------

### VECTOR结构体

### LIST结构体

### MAP结构体


## 21. 网络与通信（尚未实现）
------------------------


## 22. 程序调试语句
------------------------

目前，Doves尚未完成在诸如VSCode这样的通用IDE上的适配。但Doves提供了PAUSE语句和CONT命令用于调试程序。
在需要断点的地方，可以在程序中写入PAUSE语句。程序执行到PAUSE语句时，会暂停执行，并显示命令提示符"]"。在提示符"]"之后，可以输入PRINT语句打印当前环境下的变量、使用赋值语句改变变量值等。
而后，可以输入CONT命令让程序继续运行，或者输入END语句让程序停止。
在"]"提示符下还可输入SOURCE命令列出当前模块的源代码。

## 附录1. 错误代码表
------------------------

|内部标识|错误码值|英文描述|中文描述|
|-------|--------|--------|-------|
|SYNTAX_ERROR               |0|Syntax error.||
|INVALID_IDENTIFIER         |1|Invalid variable name.||
|CONSTANT_NOT_MUTABLE       |2|Constant can be assigned for one time.||
|UNSUPPORTED_REFERENCE      |3|Unsupported reference.||
|CANNOT_CONVERT_DATATYPE    |4|Cannot convert the data desired type.||
|VALUE_OUT_OF_RANGE         |5|Value out of range.||
|EOL_EXPECTED               |6|There should be end of line at current position.||
|SINGLE_QUOTATION_MARK_EXPECTED  |7|There should be a single quotation mark.||
|QUOTATION_MARK_EXPECTED    |8|There should be a quotation mark.||
|EXPONENT_EXPECTED          |9|There should be an exponent part of a floating point literal
|PARAM_EXPECTED             |10|Parameters are not all specified when calling a function.||
|TYPE_MISMATCH              |11|Types not match each other when comparing two variables.||
|DIVIDE_BY_ZEOR             |12|Divided by zero.||
|RIGHT_BRACE_EXPECTED       |13|Right brace expected
|IDENTIFIER_NOT_DEFINED     |14|Identifier not defined.||
|LEFT_BRACE_EXPECTED        |15|Function or method should start by a left brace
|TOO_MANY_PARAMS            |16|Function param count should be less than 100.||
|INDEX_OUT_OF_RANGE         |17|Index out of range when visiting an element of an array structure tuple etc.||
|RIGHT_SQBRACKET_EXPECTED   |18|There should be a right square bracket when visiting an element of an array 
|MEMBER_NOT_EXIST           |19|Try to visit a non-exist member of a structure.||
|CANNOT_REDEFINE_FUNC       |20|Function definition should be unique.||
|UNKNOW_DATATYPE            |21|Unknown data type while trying to convert a WordReader datatype to VarType
|ONE_STATEMENT_ONLY         |22|There should be only one statement in one line.||
|CANNOT_OPEN_SOURCE_CODE    |23|Unable to open the source code file and read.||
|PARAM_NAME_SHOULD_BE_UNIQUE|24|Parameter names of a function should be unique.||
|CANNOT_REDEFINE_STRUCTURE  |25|Structure definition should be unique.||
|PARAM_NOT_DEFINED          |26|The parameter name used here is not defined in the function body.||
|NONE_PRINTABLE_DATA        |27|Try to print something that is not printable.||
|NONE_INPUTABLE_TYPE        |28|Try to input to a variable type that is not inputable.||
|INPUT_CHAR_REQUIRED        |29|Input statement requires a character.||
|INPUT_INTEGER_REQUIRED     |30|Input statement requires an integer.||
|INPUT_BOOLEAN_REQUIRED     |31|Input statement requires a boolean (T true 1 / F false 0)
|VAR_REDFINED               |32|The variable has defined before.||
|ARRAY_SIZE_REQUIRED        |33|There should be an integer value to define the size of an array.||
|UNDEFINED_TYPE             |34|Try to declare a variable as an undefined type.||
|CANNOT_REFER_TO_DIFFERENT_TYPE  |35|Try to set a variable as a reference to another variable but the types do not match.||
|RVALUE_NOT_REFERENCABLE    |36|Cannot refer to an expression result that is not a variable.||
|USING_NOT_INITIALIZED_REF  |37|Using a reference variable that is not initialized.||
|MUST_BE_REFERENCE          |38|A structure member of the same type of the structure itself should always be a reference. Add 'BYREF'.
|FIXED_ARRAY_CANNOT_BE_REF  |39|An array itself can not refer to another array.||
|NOT_A_REF_VAR              |40|Only a reference variable can refer to another variable.||
|IF_WITHOUT_ENDIF           |41|No ENDIF nor ELSE found in the IF statement.||
|FOR_WITHOUT_INDEX_VAR      |42|'FOR' should followed by a variable assignment to be used as loop controlling variable.||
|FOR_WITHOUT_TO             |43|'FOR' without 'TO'
|NEXT_WITHOUT_FOR           |44|'NEXT/ENDFOR' without 'FOR'
|DIFF_ST_CANNOT_CROSS       |45|Try to go across different statement structures.||
|STEP_IS_ZERO               |46|Step cannot be 0.||
|ENDWHILE_WITHOUT_WHILE     |47|'ENDWHILE/WEND' without 'WHILE'.||
|FOREACH_WITHOUT_IN         |48|'FOREACH' without 'IN'.||
|FOREACH_WITHOUT_VAR        |49|'FOREACH' should be followed by a variable.||
|NOT_INDEXABLE              |50|The variable used or to be iterated is not indexable nor iteratable.||
|ENDFOREACH_WITHOUT_FOREACH |51|'ENDFOREACH' without 'FOREACH'
|SELECT_WITHOU_CASE         |52|'SELECT' must be followed by 'CASE'
|CASE_WITHOUT_SELECT        |53|There should be a 'SELECT CASE' statement before this 'CASE'.||
|UNSUPPROTE_TYPE_IN_CASE    |54|Only numerics and strings are support in 'select case'.||
|NO_TEMP_VAR_AVLB           |55|No room to create a temp variable.|| Temp vars exceed 999.||
|ENDSELECT_WITHOUT_SELECT   |56|'ENDSELECT' without 'SELECT'.||
|ENDTRY_WITHOUT_TRY         |57|'ENDTRY' or 'CATCH' without 'TRY'.||
|THROW_NOT_CATCHED          |58|Error thrown but not catched in any catch block.||
|ENDLOOP_WITHOUT_LOOP       |59|'ENDLOOP' without 'LOOP'
|BREAK_WITHOUT_LOOP         |60|'BREAK' should only be inside a loop.||
|RETRY_NOT_IN_CATCH         |61|'RETRY' should only be inside a CATCH block.||
|MEMBER_DEFINED_IN_PARENT   |62|Member has been defined in one of the parents.||
|NOT_REDIMABLE              |63|Only an array or memory area can be re-dimmed.||
|IMPORT_NAME_USED           |64|Try to import a module as a used name.||
|IDENTIFIER_TOO_LONG        |65|An identifier should not exceed 53 charactors.||
|MEMORY_SHOULD_BE_ARRAY     |66|Memory can only be defined as an array.||
|FAIL_LOADING_LIB           |67|Fail loading the library.||


## 附录2. 内置函数
------------------------

|英文定义|中文定义|描述|
|-------|-------|----|
|ASC(str AS STRING) AS INTEGER||取str第一个字符的内码|
|CHR(ch AS INTEGER) AS STRING||返回内码为ch的字符串|
|VAL(str AS STRING) AS REAL||将str的内容转换成数值|
|LEN(str AS STRING) AS INTEGER||返回字符串长度|
|TOUPPER(str AS STRING) AS STRING||将字符串转换为大写|
|TOLOWER(str AS STRING) AS STRING||将字符串转换为小写|
|LEFT(str AS STRING, length AS INTEGER) AS STRING||取字符串左边length个字符|
|RIGHT(str AS STRING, length AS INTEGER) AS STRING||取字符串右边length个字符|
|MID(str AS STRING, start AS INTEGER, length AS INTEGER) AS STRING||从字符串的第start个字符起，取length个字符|
|FIND(str AS STRING, sub AS STRING, from AS INTEGER) AS INTEGER||从字符串中的第from个位置起，找sub字串所在位置。如果找不到，返回-1|
|REPLACE(str AS STRING, s)||尚未实现|
|MAX(a, b)||取a,b中的大数|
|MIN(a,b)||取a,b中的小数|
|SIN(a AS REAL) AS REAL||求正弦值|
|COS(a AS REAL) AS REAL||求余弦值|
|TAN(a AS REAL) AS REAL||求正切值|
|ASIN(a AS REAL) AS REAL||求反正弦值|
|ACOS(a AS REAL) AS REAL||求反余弦值|
|ATAN(a AS REAL) AS REAL||求反正切值|
|LOG(a AS REAL) AS REAL||求e的对数值|
|LOG10(a AS REAL) AS REAL||求10的对数值|
|SQRT(a AS REAL) AS REAL||求平方根|
|CEILING(a AS REAL) AS INTEGER||取浮点数的上限整数值|
|FLOOR(a AS REAL) AS INTEGER||取浮点数的整数部分|
|ROUND(a AS REAL) AS INTEGER||求浮点数的四舍五入整数|
|SEED()||随机数播种|
|RAND() AS REAL||返回[0,1]之间的随机数|
|SETW(w AS INTEGER)||指定下一个PRINT内容的宽度|
|SETPRECISION(p AS INTEGER)||指定下一个PRINT浮点数的精度|
|TAB(s AS INTEGER)||指定下一个打印的列位置（需终端支持，Windows可能不支持）|
|TICKS() AS INTEGER||返回从程序运行开始到目前为止的毫秒计数|
|TYPENAME(a) AS STRING||返回a的类型|
|ISNUMERIC(a) AS BOOLEAN||返回a是否是数值类型|
|ISSTRING(a) AS BOOLEAN||返回a是否是字符串|
|ISARRAY(a) AS BOOLEAN||返回a是否是数组|
|ISBOOLEAN(a) AS BOOLEAN||返回a是否是布尔型|
|ISNULL(a) AS BOOLEAN||返回a是否是空类型|
|ISSTRUCTUREOF(a, s AS STRING) AS BOOLEAN||返回s是否是结构体s|
|SLEEP(a AS INTEGER)||等待a毫秒|
|GETREFCOUNT(a) AS INTEGER||取a的引用计数|
|ARRAYSIZE(a AS ARRAY) AS INTEGER||取数组a的长度|


## 附录3. 关于... About Doves...
------------------------

我在20多年前学编译系统原理课程时，就做了一个解释型语言。当然，没啥实际的用处，只在我后来的软件中用它完成了一些表达式解析和简单的脚本嵌入工作。随着我儿子渐渐掌握计算机编程能力，我发现有必要给孩子们写一套既简单又能真正工作的编程工具，既不是scratch那样的玩具，又不是c++那么晦涩。于是我就想着把我数十年前的代码拿出来，补充、完善一下，改造成实用的计算机语言。
It has been more than 20 years since I made a computer language interpreter after the "compiler principles" course. It was not quite useful even though I embedded it in some of my softwares to parse expressions. Things changed when my son started learning programming and I realized that it would be great if there is a simple programming language for children, not a toy like Scratch, nor as complicated as c++. I thought of my decades old interpreter and pulled it out, polished it to be something practical.

这个语言项目原本叫“水管”——我儿子小时候见到能出水的水管就走不动地。我跟我儿子说，你来给我的计算机语言起个名字吧？小家伙想了想，说，叫“鸽子”如何？我查了字典，复数形式的鸽子——Doves，既是鸽子，也是圣灵的意思。这个典故源于圣经：“耶稣受了洗，随即从水里上来。天忽然为他开了，他就看见神的灵仿佛鸽子降下，落在他身上。从天上有声音说：“这是我的爱子，我所喜悦的。”（马太福音3:16-17，中文和合本）
I named this project "Waterpipe", which had been my son's favorite before he went to school. I asked my son to give it a better name, and he replied "Dove". I found the word "Doves" also means Spirit and has the origin of the Bible: As soon as Jesus was baptized, he went up out of the water. At that moment heaven was opened, and he save the Spirit of God descending like a dove and alignting on him. And a voice from heaven said, "This is my Son, whom I love; with him I am well pleased." (Matt 3:16-17, NIV)

那么，就叫“Doves”吧。
And, good, it is "Doves".

