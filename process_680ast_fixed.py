#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import re
import os

def process_ast_file(input_file, output_file):
    """
    处理AST文件，将包含Print Tree的行替换为指定格式
    
    Args:
        input_file (str): 输入文件路径
        output_file (str): 输出文件路径
    """
    try:
        # 检查文件是否存在
        if not os.path.exists(input_file):
            print(f"错误：文件不存在 - {input_file}")
            print(f"当前工作目录：{os.getcwd()}")
            print(f"绝对路径：{os.path.abspath(input_file)}")
            return False
        
        # 检查文件是否可读
        if not os.access(input_file, os.R_OK):
            print(f"错误：文件不可读 - {input_file}")
            return False
        
        print(f"正在读取文件：{input_file}")
        
        # 尝试不同的编码方式
        encodings = ['utf-8', 'gbk', 'gb2312', 'latin-1']
        lines = None
        
        for encoding in encodings:
            try:
                with open(input_file, 'r', encoding=encoding) as f:
                    lines = f.readlines()
                print(f"成功使用编码：{encoding}")
                break
            except UnicodeDecodeError:
                print(f"编码 {encoding} 失败，尝试下一个...")
                continue
            except Exception as e:
                print(f"读取文件时出错（编码 {encoding}）：{e}")
                continue
        
        if lines is None:
            print("错误：无法使用任何编码读取文件")
            return False
        
        print(f"文件读取成功，共 {len(lines)} 行")
        
        processed_lines = []
        i = 0
        
        while i < len(lines):
            line = lines[i]
            
            # 检查是否包含Print Tree
            if 'Print Tree' in line:
                # 获取当前行的缩进
                indent = len(line) - len(line.lstrip())
                
                # 检查下一行是否存在
                if i + 1 < len(lines):
                    next_line = lines[i + 1]
                    
                    # 解析下一行获取LABEL和LOC
                    label, loc = parse_next_line(next_line)
                    
                    if label and loc:
                        # 创建新的行，缩进减少2个空格
                        new_indent = max(0, indent - 2)
                        new_line = ' ' * new_indent + f'-> {{ ({label})   ({loc})\n'
                        processed_lines.append(new_line)
                        
                        # 跳过下一行（因为我们已经处理了它）
                        i += 2
                        continue
                    else:
                        # 如果解析失败，保留原行
                        processed_lines.append(line)
                        i += 1
                else:
                    # 如果没有下一行，保留原行
                    processed_lines.append(line)
                    i += 1
            else:
                # 不包含Print Tree的行直接保留
                processed_lines.append(line)
                i += 1
        
        # 检查输出目录是否存在
        output_dir = os.path.dirname(output_file)
        if output_dir and not os.path.exists(output_dir):
            print(f"创建输出目录：{output_dir}")
            os.makedirs(output_dir, exist_ok=True)
        
        # 写入输出文件
        print(f"正在写入文件：{output_file}")
        with open(output_file, 'w', encoding='utf-8') as f:
            f.writelines(processed_lines)
        
        print(f"成功处理文件：")
        print(f"  输入文件：{input_file}")
        print(f"  输出文件：{output_file}")
        print(f"  处理行数：{len(processed_lines)}")
        
        return True
        
    except FileNotFoundError:
        print(f"错误：文件 {input_file} 不存在")
        print(f"当前工作目录：{os.getcwd()}")
        print(f"绝对路径：{os.path.abspath(input_file)}")
        return False
    except PermissionError:
        print(f"错误：没有权限访问文件 {input_file}")
        return False
    except Exception as e:
        print(f"错误：{e}")
        print(f"错误类型：{type(e).__name__}")
        import traceback
        traceback.print_exc()
        return False

def parse_next_line(line):
    """
    解析下一行，提取LABEL和LOC
    
    Args:
        line (str): 下一行的内容
        
    Returns:
        tuple: (label, loc) 或 (None, None) 如果解析失败
    """
    try:
        # 去除前导空格
        line = line.strip()
        
        # 查找箭头和括号
        arrow_match = re.search(r'->\s*(\w+)', line)
        if not arrow_match:
            return None, None
        
        label_type = arrow_match.group(1)
        
        # 根据LABEL类型确定DEF类型
        if label_type == 'BUNDLE_LABEL':
            label = 'BUNDLE_DEF'
        elif label_type == 'FUNCTION_LABEL':
            label = 'FUNCTION_DEF'
        else:
            # 对于其他类型，使用原类型
            label = label_type
        
        # 提取LOC信息（在括号中的内容）
        loc_match = re.search(r'\(([^)]+)\)', line)
        if loc_match:
            loc = loc_match.group(1)
        else:
            loc = ""
        
        return label, loc
        
    except Exception as e:
        print(f"解析行时出错：{e}")
        return None, None

def main():
    """主函数"""
    if len(sys.argv) != 3:
        print("用法：python process_680ast_fixed.py <输入文件> <输出文件>")
        print("示例：python process_680ast_fixed.py input.ast output.ast")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"开始处理...")
    print(f"输入文件：{input_file}")
    print(f"输出文件：{output_file}")
    
    success = process_ast_file(input_file, output_file)
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main() 