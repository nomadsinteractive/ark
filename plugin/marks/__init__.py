"""
My Ark Studio module
"""

from ark import loge


def pydevd_start(host: str = 'localhost', port: int = 5678):
    try:
        import pydevd
        pydevd.settrace(host, port=port, stdoutToServer=True, stderrToServer=True, suspend=False)
    except Exception as e:
        loge(e)


def pydevd_stop():
    import pydevd
    pydevd.stoptrace()
