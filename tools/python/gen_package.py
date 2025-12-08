import getopt
import sys
import zipfile
import os


def main():
    opts, paths = getopt.getopt(sys.argv[1:], 'o:')
    params = dict((i.lstrip('-'), j) for i, j in opts)

    output_zip = params.get('o', 's01.ark')
    for i in paths:
        with zipfile.ZipFile(output_zip, 'w', compression=zipfile.ZIP_DEFLATED) as zfp:
            for root, _, files in os.walk(i, followlinks=True):
                for j in files:
                    file_path = os.path.join(root, j)
                    arcname = os.path.relpath(file_path, i)
                    zfp.write(file_path, arcname)


if __name__ == '__main__':
    main()
