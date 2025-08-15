#!/usr/bin/env bash

# 文件夹分组脚本（复制版本）
# 使用方法: ./split_folders_copy.sh [源文件夹路径] [每组的数量，默认100] [是否移动，默认false]

# 获取脚本所在目录
SCRIPT_DIR=$(cd $(dirname $BASH_SOURCE[0]) && pwd)
REPO_DIR=$(cd $SCRIPT_DIR/.. && pwd)

# 默认参数
SOURCE_DIR=${1:-"$REPO_DIR/data"}
GROUP_SIZE=${2:-100}
MOVE_MODE=${3:-false}

# 检查源文件夹是否存在
if [ ! -d "$SOURCE_DIR" ]; then
    echo "错误：源文件夹不存在: $SOURCE_DIR"
    echo "使用方法: $0 [源文件夹路径] [每组的数量] [是否移动]"
    echo "示例: $0 ./data 100 false  # 复制模式"
    echo "示例: $0 ./data 100 true   # 移动模式"
    exit 1
fi

# 获取源文件夹名称
SOURCE_DIR_NAME=$(basename "$SOURCE_DIR")
SOURCE_DIR_PARENT=$(dirname "$SOURCE_DIR")

# 确定操作模式
if [ "$MOVE_MODE" = "true" ]; then
    OPERATION="移动"
    COPY_CMD="mv"
else
    OPERATION="复制"
    COPY_CMD="cp -r"
fi

echo "开始处理文件夹分组..."
echo "源文件夹: $SOURCE_DIR"
echo "源文件夹名称: $SOURCE_DIR_NAME"
echo "每组数量: $GROUP_SIZE"
echo "操作模式: $OPERATION"
echo "=================================="

# 获取所有子文件夹
subdirs=()
# 使用更兼容的方法，避免子shell隔离问题
cd "$SOURCE_DIR" || exit 1
for dir in */; do
    if [ -d "$dir" ]; then
        # 移除末尾的斜杠
        dir=${dir%/}
        subdirs+=("$SOURCE_DIR/$dir")
    fi
done
cd - > /dev/null || exit 1

# 计算子文件夹总数
total_dirs=${#subdirs[@]}
echo "发现 $total_dirs 个子文件夹"

if [ $total_dirs -eq 0 ]; then
    echo "没有找到子文件夹，退出"
    exit 0
fi

# 计算需要多少个分组
num_groups=$(( (total_dirs + GROUP_SIZE - 1) / GROUP_SIZE ))
echo "将分成 $num_groups 个组"

# 创建分组
for ((group=1; group<=num_groups; group++)); do
    # 计算当前组的起始和结束索引
    start_idx=$(( (group - 1) * GROUP_SIZE ))
    end_idx=$(( group * GROUP_SIZE - 1 ))
    
    # 确保不超出数组范围
    if [ $end_idx -ge $total_dirs ]; then
        end_idx=$((total_dirs - 1))
    fi
    
    # 创建目标文件夹名称
    target_dir="$SOURCE_DIR_PARENT/${SOURCE_DIR_NAME}-$group"
    
    echo "创建分组 $group: $target_dir (包含文件夹 $((start_idx + 1))-$((end_idx + 1)))"
    
    # 创建目标文件夹
    mkdir -p "$target_dir"
    
    # 复制或移动文件夹
    for ((i=start_idx; i<=end_idx; i++)); do
        if [ $i -lt $total_dirs ]; then
            dir_name=$(basename "${subdirs[$i]}")
            echo "  $OPERATION: $dir_name"
            $COPY_CMD "${subdirs[$i]}" "$target_dir/"
        fi
    done
    
    # 统计当前组中的文件夹数量
    actual_count=$(find "$target_dir" -maxdepth 1 -type d | wc -l)
    actual_count=$((actual_count - 1))  # 减去目标文件夹本身
    echo "  分组 $group 完成，包含 $actual_count 个文件夹"
done

echo "=================================="
echo "分组完成！"
echo "总共处理了 $total_dirs 个文件夹"
echo "分成了 $num_groups 个组"
echo "操作模式: $OPERATION"
echo "输出文件夹:"
for ((group=1; group<=num_groups; group++)); do
    target_dir="$SOURCE_DIR_PARENT/${SOURCE_DIR_NAME}-$group"
    if [ -d "$target_dir" ]; then
        count=$(find "$target_dir" -maxdepth 1 -type d | wc -l)
        count=$((count - 1))  # 减去目标文件夹本身
        echo "  $target_dir ($count 个文件夹)"
    fi
done

if [ "$MOVE_MODE" != "true" ]; then
    echo ""
    echo "注意：原始文件夹已保留。如需移动模式，请使用："
    echo "  $0 $SOURCE_DIR $GROUP_SIZE true"
fi 