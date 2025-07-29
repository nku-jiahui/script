import os
import re
import sys

# 配置
START_MARKER = ('xxx', 'yyy')
OUTPUT_DIR = 'output'
HEADER_LINE = 'zzz'
INDENT = '  '  # 两个空格

def extract_function_name(line):
    """
    从形如 '-> nameabc (IDENT)' 的行中提取 nameabc
    """
    match = re.search(r'->\s+(\w+)\s+$IDENT$', line)
    if match:
        return match.group(1)
    else:
        return None

def process_input_file(input_file, output_dir=OUTPUT_DIR):
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    print(f"📁 确保输出目录存在：{output_dir}")

    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = [line.rstrip('\n') for line in f.readlines()]
    except FileNotFoundError:
        print(f"❌ 错误：输入文件 '{input_file}' 不存在！")
        return
    except Exception as e:
        print(f"❌ 读取文件时出错：{e}")
        return

    # 存储每个函数的 (name, content) 列表
    functions = []
    i = 0
    n = len(lines)

    while i < n:
        line = lines[i].strip()

        if line in START_MARKER:
            # 当前是 xxx 或 yyy，开始一个新函数
            start_i = i
            func_lines = []

            # 下一行是 func_start 行（可能不需要）
            if i + 1 < n:
                i += 1
                # 再下一行是 -> name (IDENT)
                if i + 1 < n:
                    i += 1
                    name_line = lines[i]
                    func_name = extract_function_name(name_line)
                    if not func_name:
                        print(f"⚠️ 警告：在第 {i+1} 行无法提取函数名：{name_line}")
                        func_name = f"unknown_{len(functions)}"
                    func_lines.append(name_line)
                else:
                    print(f"⚠️ 警告：'xxx/yyy' 后缺少行，跳过")
                    i += 1
                    continue
            else:
                print("⚠️ 警告：文件以 xxx/yyy 结尾，无内容")
                break

            # 收集后续行，直到遇到下一个 xxx 或 yyy
            i += 1
            while i < n and lines[i].strip() not in START_MARKER:
                func_lines.append(lines[i])
                i += 1

            # 处理缩进 + 添加 zzz 头部
            indented_content = [INDENT + content_line for content_line in func_lines]
            indented_content.insert(0, HEADER_LINE)  # 在最前面插入 zzz

            # 保存
            functions.append((func_name, indented_content))
            print(f"✅ 提取函数：{func_name}")

        else:
            i += 1

    # 写入每个函数到独立文件
    for func_name, content_lines in functions:
        output_file = os.path.join(output_dir, f"{func_name}.txt")
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write('\n'.join(content_lines) + '\n')
            print(f"📄 已写入：{output_file}")
        except Exception as e:
            print(f"❌ 写入文件 {output_file} 失败：{e}")

    print(f"🎉 完成！共提取 {len(functions)} 个函数到 '{output_dir}' 目录。")

# ============ 主程序入口 ============
if __name__ == "__main__":
    if len(sys.argv) == 2:
        input_file = sys.argv[1]
    else:
        input_file = "input.txt"
        print(f"📌 使用默认输入文件：{input_file}")
        print(f"📌 用法：python {sys.argv[0]} <输入文件路径>")

    process_input_file(input_file)