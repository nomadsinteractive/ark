"""
My Ark Studio module
"""

from ark import loge


def pydevd_start(host: str = 'localhost', port: int = 56789):
    if pydevd := import_pydevd_module():
        try:
            pydevd.settrace(host, port=port, stdoutToServer=True, stderrToServer=True, suspend=False)
        except Exception as e:
            loge(e)


def pydevd_stop():
    if pydevd := import_pydevd_module():
        pydevd.stoptrace()


def import_pydevd_module():
    try:
        import pydevd_pycharm
        return pydevd_pycharm
    except ImportError:
        try:
            import pydevd
            return pydevd
        except ImportError:
            return None
