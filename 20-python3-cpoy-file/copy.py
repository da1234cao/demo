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
