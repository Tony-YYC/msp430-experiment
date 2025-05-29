# 创建BGR格式的图像数组提取工具，支持导出C头文件
import numpy as np
from PIL import Image
import argparse
import os
import sys

def extract_bgr_array(image_path):
    """
    从图像文件中提取BGR数组数据
    返回BGR数组和图像信息
    """
    try:
        # 打开图像
        with Image.open(image_path) as img:
            original_mode = img.mode
            file_size = os.path.getsize(image_path)
            
            # 转换为RGB模式（如果不是的话）
            if img.mode != 'RGB':
                img = img.convert('RGB')
            
            # 获取图像信息
            width, height = img.size
            
            # 转换为numpy数组 (RGB格式)
            rgb_array = np.array(img)
            
            # 转换为BGR格式
            bgr_array = rgb_array[:, :, ::-1]  # 反转颜色通道顺序 RGB -> BGR
            
            # 计算行字节对齐
            bytes_per_pixel = 3  # BGR每像素3字节
            row_bytes = width * bytes_per_pixel
            aligned_row_bytes = ((row_bytes + 3) // 4) * 4  # 对齐到4字节边界
            padding_bytes = aligned_row_bytes - row_bytes
            
            # 如果需要填充，添加填充字节
            if padding_bytes > 0:
                # 为每行添加填充
                padded_array = np.zeros((height, aligned_row_bytes), dtype=np.uint8)
                for i in range(height):
                    # 复制BGR数据
                    padded_array[i, :row_bytes] = bgr_array[i].flatten()
                    # 填充字节保持为0
                bgr_data = padded_array
            else:
                # 不需要填充，直接展平
                bgr_data = bgr_array.reshape(height, -1)
            
            info = {
                'original_mode': original_mode,
                'file_size': file_size,
                'width': width,
                'height': height,
                'total_pixels': width * height,
                'row_bytes': row_bytes,
                'aligned_row_bytes': aligned_row_bytes,
                'padding_bytes': padding_bytes,
                'total_bytes': height * aligned_row_bytes,
                'memory_mb': (height * aligned_row_bytes) / (1024 * 1024)
            }
            
            return bgr_data, bgr_array, info
            
    except Exception as e:
        print(f"错误：无法处理图像文件 '{image_path}': {e}")
        return None, None, None

def analyze_bgr_array(bgr_array, info):
    """分析BGR数组并显示统计信息"""
    print("\n" + "="*60)
    print("🖼️  BGR图像数组分析报告")
    print("="*60)
    
    # 基本信息
    print(f"📋 基本信息:")
    print(f"   原始图像模式: {info['original_mode']}")
    print(f"   文件大小: {info['file_size']:,} 字节")
    print(f"   图像尺寸: {info['width']} × {info['height']} 像素")
    print(f"   像素总数: {info['total_pixels']:,}")
    print(f"   颜色格式: BGR (蓝-绿-红)")
    
    print(f"\n📐 内存布局:")
    print(f"   每行原始字节数: {info['row_bytes']} 字节")
    print(f"   对齐后每行字节数: {info['aligned_row_bytes']} 字节")
    print(f"   每行填充字节数: {info['padding_bytes']} 字节")
    print(f"   总字节数: {info['total_bytes']:,} 字节")
    print(f"   内存占用: {info['memory_mb']:.2f} MB")
    
    # 通道统计（基于原始BGR数组，不包含填充）
    if bgr_array is not None:
        print(f"\n📊 BGR通道统计:")
        channels = ['蓝色(B)', '绿色(G)', '红色(R)']
        for i, channel in enumerate(channels):
            channel_data = bgr_array[:, :, i]
            print(f"   {channel}:")
            print(f"      最小值: {np.min(channel_data)}")
            print(f"      最大值: {np.max(channel_data)}")
            print(f"      平均值: {np.mean(channel_data):.1f}")
            print(f"      中位数: {np.median(channel_data):.1f}")
            print(f"      标准差: {np.std(channel_data):.1f}")
        
        # 显示左上角像素示例
        print(f"\n🔍 左上角5×5像素BGR值示例:")
        sample_size = min(5, info['height'], info['width'])
        for i in range(sample_size):
            row_values = []
            for j in range(sample_size):
                b, g, r = bgr_array[i, j]
                row_values.append(f"({b:3d},{g:3d},{r:3d})")
            print(f"   行{i}: {' '.join(row_values)}")

def save_as_c_header(bgr_data, info, output_path, array_name=None):
    """将BGR数据保存为C头文件格式"""
    if array_name is None:
        # 从输出文件名生成数组名
        base_name = os.path.splitext(os.path.basename(output_path))[0]
        array_name = base_name.replace('-', '_').replace(' ', '_')
        if not array_name.isidentifier():
            array_name = "image_data"
    
    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            # 写入头文件保护
            header_guard = f"{array_name.upper()}_H"
            f.write(f"#ifndef {header_guard}\n")
            f.write(f"#define {header_guard}\n\n")
            
            # 写入注释和信息
            f.write("/*\n")
            f.write(" * BGR图像数据数组\n")
            f.write(f" * 图像尺寸: {info['width']} × {info['height']} 像素\n")
            f.write(f" * 颜色格式: BGR (蓝-绿-红)\n")
            f.write(f" * 每行字节数: {info['aligned_row_bytes']} 字节 (4字节对齐)\n")
            f.write(f" * 填充字节数: {info['padding_bytes']} 字节/行\n")
            f.write(f" * 总字节数: {info['total_bytes']} 字节\n")
            f.write(" */\n\n")
            
            # 写入宏定义
            f.write(f"#define {array_name.upper()}_WIDTH  {info['width']}\n")
            f.write(f"#define {array_name.upper()}_HEIGHT {info['height']}\n")
            f.write(f"#define {array_name.upper()}_ROW_BYTES {info['aligned_row_bytes']}\n")
            f.write(f"#define {array_name.upper()}_TOTAL_BYTES {info['total_bytes']}\n\n")
            
            # 写入数组声明
            f.write(f"const unsigned char {array_name}[{info['total_bytes']}] = {{\n")
            
            # 写入数据
            flat_data = bgr_data.flatten()
            for i in range(0, len(flat_data), 16):  # 每行16个字节
                line_data = flat_data[i:i+16]
                hex_values = [f"0x{byte:02X}" for byte in line_data]
                f.write(f"    {', '.join(hex_values)}")
                if i + 16 < len(flat_data):
                    f.write(",")
                f.write(f"  // 偏移 0x{i:04X}\n")
            
            f.write("};\n\n")
            f.write(f"#endif // {header_guard}\n")
        
        print(f"✅ C头文件已保存到: {output_path}")
        print(f"   数组名称: {array_name}")
        print(f"   数组大小: {info['total_bytes']} 字节")
        
    except Exception as e:
        print(f"❌ 保存C头文件失败: {e}")

def save_bgr_data(bgr_data, bgr_array, info, output_path, format_type='npy'):
    """保存BGR数据到文件"""
    try:
        if format_type == 'npy':
            # 保存原始BGR数组（不含填充）
            np.save(output_path, bgr_array)
            print(f"✅ NumPy数组已保存到: {output_path}")
            
        elif format_type == 'csv':
            # 保存为CSV格式（原始BGR数组）
            reshaped = bgr_array.reshape(-1, 3)
            np.savetxt(output_path, reshaped, delimiter=',', fmt='%d', 
                      header='B,G,R', comments='')
            print(f"✅ CSV文件已保存到: {output_path}")
            
        elif format_type == 'bin':
            # 保存为二进制文件（包含填充的对齐数据）
            bgr_data.astype(np.uint8).tofile(output_path)
            print(f"✅ 二进制文件已保存到: {output_path}")
            print(f"   包含4字节对齐填充数据")
            
        elif format_type == 'txt':
            # 保存为文本文件
            with open(output_path, 'w') as f:
                f.write(f"BGR图像数据\n")
                f.write(f"尺寸: {info['width']} × {info['height']}\n")
                f.write(f"每行字节数: {info['aligned_row_bytes']} (含填充)\n")
                f.write(f"总字节数: {info['total_bytes']}\n\n")
                
                # 写入像素数据（原始BGR，不含填充）
                f.write("BGR像素数据 (B,G,R):\n")
                for i in range(min(10, info['height'])):  # 只显示前10行
                    f.write(f"行 {i}: ")
                    for j in range(min(10, info['width'])):  # 只显示前10列
                        b, g, r = bgr_array[i, j]
                        f.write(f"({b},{g},{r}) ")
                    f.write("\n")
                if info['height'] > 10 or info['width'] > 10:
                    f.write("...(省略其余数据)\n")
            
            print(f"✅ 文本文件已保存到: {output_path}")
            
    except Exception as e:
        print(f"❌ 保存文件失败: {e}")

def get_pixel_bgr(bgr_array, x, y, info):
    """获取指定位置的BGR值"""
    if 0 <= x < info['width'] and 0 <= y < info['height']:
        b, g, r = bgr_array[y, x]  # 注意：数组是[行,列]
        print(f"\n📍 位置 ({x}, {y}) 的BGR值:")
        print(f"   蓝色(B): {b}")
        print(f"   绿色(G): {g}")
        print(f"   红色(R): {r}")
        print(f"   十六进制: 0x{b:02X}{g:02X}{r:02X}")
        return b, g, r
    else:
        print(f"❌ 坐标 ({x}, {y}) 超出图像范围 (0-{info['width']-1}, 0-{info['height']-1})")
        return None

def main():
    parser = argparse.ArgumentParser(description='从图像提取BGR数组数据并导出为C头文件')
    parser.add_argument('input', nargs='?', help='输入图像文件路径')
    parser.add_argument('-o', '--output', help='输出文件路径')
    parser.add_argument('-f', '--format', choices=['npy', 'csv', 'txt', 'bin', 'c'], 
                       default='c', help='输出格式 (默认: c)')
    parser.add_argument('--array-name', help='C头文件中的数组名称')
    parser.add_argument('--no-analysis', action='store_true', help='跳过数组分析报告')
    parser.add_argument('--pixel', nargs=2, type=int, metavar=('X', 'Y'), 
                       help='获取指定坐标的BGR值')
    
    args = parser.parse_args()
    
    # 如果没有提供输入文件，进入交互模式
    if not args.input:
        print("🖼️  BGR图像数组提取工具")
        print("支持导出C头文件用于单片机开发\n")
        
        image_path = input("请输入图像文件路径: ").strip().strip('"')
        if not image_path:
            print("❌ 未提供图像文件路径")
            return
        
        output_path = input("请输入输出文件路径 (留空则自动生成): ").strip().strip('"')
        
        format_choice = input("选择输出格式 [c/npy/csv/txt/bin] (默认: c): ").strip().lower()
        if not format_choice:
            format_choice = 'c'
        
        array_name = None
        if format_choice == 'c':
            array_name = input("请输入C数组名称 (留空则自动生成): ").strip()
    else:
        image_path = args.input
        output_path = args.output
        format_choice = args.format
        array_name = args.array_name
    
    # 检查输入文件
    if not os.path.exists(image_path):
        print(f"❌ 文件不存在: {image_path}")
        return
    
    # 提取BGR数组
    print(f"🔄 正在处理图像: {image_path}")
    bgr_data, bgr_array, info = extract_bgr_array(image_path)
    
    if bgr_data is None:
        return
    
    # 显示分析报告
    if not (hasattr(args, 'no_analysis') and args.no_analysis):
        analyze_bgr_array(bgr_array, info)
    
    # 处理像素查询
    if hasattr(args, 'pixel') and args.pixel:
        x, y = args.pixel
        get_pixel_bgr(bgr_array, x, y, info)
    
    # 保存文件
    if output_path or not hasattr(args, 'input') or args.input:
        if not output_path:
            # 自动生成输出文件名
            base_name = os.path.splitext(os.path.basename(image_path))[0]
            if format_choice == 'c':
                output_path = f"{base_name}_bgr.h"
            else:
                extensions = {'npy': '.npy', 'csv': '.csv', 'txt': '.txt', 'bin': '.bin'}
                output_path = f"{base_name}_bgr{extensions[format_choice]}"
        
        if format_choice == 'c':
            save_as_c_header(bgr_data, info, output_path, array_name)
        else:
            save_bgr_data(bgr_data, bgr_array, info, output_path, format_choice)

if __name__ == "__main__":
    main()