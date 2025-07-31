#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import re

def process_ast_file(input_file, output_file):
    """
    处理AST文件，将包含Print Tree的行替换为指定格式
    
    Args:
        input_file (str): 输入文件路径
        output_file (str): 输出文件路径
    """
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
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
        
        # 写入输出文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.writelines(processed_lines)
        
        print(f"成功处理文件：")
        print(f"  输入文件：{input_file}")
        print(f"  输出文件：{output_file}")
        
        return True
        
    except FileNotFoundError:
        print(f"错误：文件 {input_file} 不存在")
        return False
    except Exception as e:
        print(f"错误：{e}")
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
        print("用法：python process_ast.py <输入文件> <输出文件>")
        print("示例：python process_ast.py input.ast output.ast")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    success = process_ast_file(input_file, output_file)
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main() 