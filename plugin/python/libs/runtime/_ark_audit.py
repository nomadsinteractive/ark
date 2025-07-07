"""
Put your audit rules here
"""

import os.path
import sys

import ark


class Auditor:
    def __init__(self):
        self._disallowed_events = {
            'os.system', 'os.remove', 'os.exec', 'os.rmdir', 'os.link', 'os.symlink', 'os.fork', 'os.forkpty', 'subprocess.Popen',
            'socket.bind', 'socket.connect', 'socket.sendmsg', 'socket.sendto', 'http.client.connect', 'http.client.send', 'urllib.Request',
            'ctypes.dlopen', 'ctypes.dlsym', 'ctypes.dlsym/handle'
        }

    def __call__(self, event: str, args):
        if event in self._disallowed_events:
            raise PermissionError(f'Access Denied: {event}')

        try:
            func = getattr(self, event.replace('.', '_').replace('.', '_'))
        except AttributeError:
            func = None
        if func:
            func(*args)

    @staticmethod
    def open(filepath: str, mode: str, flags: int):
        if 'w' in mode:
            raise PermissionError(f'Access Denied: Writing to files is not allowed')

        dirname, _ = os.path.split(ark.facade().argv[0])
        abspath = os.path.abspath(filepath)
        if not abspath.startswith(dirname):
            raise PermissionError(f'Access Denied: Opening file out of chroot "{dirname}"')


sys.addaudithook(Auditor())