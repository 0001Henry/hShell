# hShell

hShell 实现了多个内部命令，涵盖了文件操作、环境变量管理、目录切换等功能；外部命令执行通过调用exec系统调用的方式来实现；支持NAME、PATH等系统环境变量。


## 1. `touch`
**功能**: 创建一个或多个空文件，如果文件已存在则更新其时间戳。
- **用法**: `touch [文件1] [文件2] ...`
- **示例**:
  ```bash
  touch file1.txt file2.txt
  ```

## 2. `rm`
**功能**: 删除文件或目录。如果是文件夹，支持递归删除。
- **用法**: `rm [文件/目录]`
- **示例**:
  ```bash
  rm file.txt
  rm folder
  ```

## 3. `cd`
**功能**: 切换当前工作目录。支持切换到指定目录或用户的 HOME 目录。
- **用法**: `cd [目录]`
- **示例**:
  ```bash
  cd folder
  cd ~
  ```

## 4. `pwd`
**功能**: 输出当前的工作目录路径。
- **用法**: `pwd`

## 5. `exit`
**功能**: 退出当前 shell。
- **用法**: `exit`

## 6. `echo`
**功能**: 输出给定的字符串或变量的值。
- **用法**: `echo [字符串/变量]`
- **示例**:
  ```bash
  echo hello world
  echo $HOME
  ```

## 7. `type`
**功能**: 检查命令是否为 hShell 内置命令，或是否存在于 PATH 中的外部命令。
- **用法**: `type [命令]`
- **示例**:
  ```bash
  type cd
  type ls
  ```

## 8. `env`
**功能**: 显示当前所有的环境变量。
- **用法**: `env`

## 9. `export`
**功能**: 设置或修改环境变量。
- **用法**: `export VAR=value`
- **示例**:
  ```bash
  export MYVAR=hello
  ```

## 10. `unset`
**功能**: 删除指定的环境变量。
- **用法**: `unset [变量名]`
- **示例**:
  ```bash
  unset MYVAR
  ```

## 11. `cat`
**功能**: 显示一个或多个文件的内容。支持显示行号。
- **用法**: `cat [-n] [文件]`
- **示例**:
  ```bash
  cat file.txt
  cat -n file.txt
  ```

## 12. `cp`
**功能**: 文件或文件夹的复制。
- **用法**: `cp [-i] [-p] source target`
- **示例**:
  ```bash
  cat file1.txt file2.txt
  cat -i file1.txt file2.txt
  cd -p folder1 folder2
  ```
  
  