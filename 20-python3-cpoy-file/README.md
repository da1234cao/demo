[toc]

## 前言

最近编译了一个代码，编译完之后，需要将里面的资源、库、可执行程序复制出来，便于打包。

上面需求可以“抽象”成：将指定源目录下的文件/文件夹，复制到指定目标目录。

写个脚本便可轻松完成上面需求。虽然是在linux的环境中，但为了以后在win下也可以使用，不选shell脚本，使用python就挺好。

代码需要自己写吗？ 不需要。最近[chatgpt](https://chat.openai.com/)挺火，让它写就好。（我很久不写python，脑子对python有点生疏。

chatgpt写的代码稍微有点问题，下面是微调可以满足我需求的代码。

----

## 代码

```python
import shutil
import os

def copy_files_and_dirs(src_dir, dst_dir, items):
    results = []
    for item in items:
        src_path = os.path.join(src_dir, item)
        dst_path = os.path.join(dst_dir, item)
        if os.path.isfile(src_path):
            try:
                shutil.copy(src_path, dst_path)
                results.append((item, True, ''))
            except Exception as e:
                results.append((item, False, str(e)))
        elif os.path.isdir(src_path):
            try:
                shutil.copytree(src_path, dst_path)
                results.append((item, True, ''))
            except Exception as e:
                results.append((item, False, str(e)))
        else:
            print(f'unknow : {src_path}')
            continue
    
    for item, success, reason in results:
        if success:
            print(f'{item} 复制成功')
        else:
            print(f'{item} 复制失败，原因：{reason}')


if __name__ == '__main__':
    src_dir = '/path/to/source/directory'
    dst_dir = '/path/to/destination/directory'
    items = [
        'file1.txt',
        'directory1',
        'file2.txt',
        'directory2'
    ]

    copy_files_and_dirs(src_dir, dst_dir, items)

```