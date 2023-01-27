import os
import sys


class Sign():
    def __init__(self, root_app):
        self.timestamp_servers = ['http://timestamp.verisign.com/scripts/timestamp.dll',
                                  'http://timestamp.globalsign.com/scripts/timstamp.dll',
                                  'http://timestamp.comodoca.com/authenticode',
                                  'http://www.startssl.com/timestamp']
        self.root_app = root_app
        self.file_ext_win = ['.dll', '.exe']
        self.dirs_win = []
        self.file_ext_darwin = ['.dylib']
        self.dir_darwin = ".app"

    def _sign_target(self, target, sign_cmd):

        print("Sign {}".format(target))
        ret = os.system('{} "{}"'.format(sign_cmd, target))

        return ret

    def _verify_target(self, target, verify_cmd):
        print("Verify {}".format(target))
        ret = os.system('{} "{}"'.format(verify_cmd, target))

        return ret

    def sign_macos(self):

        # Sign libs
        targets = list()

        # Define sign cmd for macOS
        codesign_cmd = "codesign --force --deep --sign 'Apple Distribution: VOXEET INC. (B55NRA8BRW)' --keychain ~/Library/Keychains/voxeet.keychain-db"
        codesign_verify_cmd = 'codesign --verify --deep --verbose=4'

        # collect targets
        for root, dirs, files in os.walk(self.root_app):
            for file_ in files:
                if os.path.splitext(file_)[-1] in self.file_ext_darwin:
                    targets += [os.path.join(root, file_)]

        targets_sorted = sorted(targets, key=lambda x: x.count(os.sep), reverse=True)

        for target in targets:
            ret_s = self._sign_target(target, codesign_cmd)

            if ret_s != 0:
                print("ERROR: {} returned {} during signing of {}. Checking another timestamp server.".format(codesign_cmd, ret_s, target))

            ret_v = self._verify_target(target, codesign_verify_cmd)

            if ret_v !=0:
                raise IOError("ERROR: {} returned {} during verifycation of {}.".format(codesign_verify_cmd, ret_v, target))

        print("Done with {} targets.".format(len(targets)))

    def sign_windows(self):
        targets = list()

        # signtool from Windows Kits 10 was added manually to env path on the Windows machine

        # collect targets
        for root, dirs, files in os.walk(self.root_app):
            for file_ in files:
                if os.path.splitext(file_)[-1] in self.file_ext_win:
                    abs_file_path = os.path.join(root, file_)
                    targets += [abs_file_path]
            for dir_ in dirs:
                if dir_ in self.dirs_win:
                    abs_dir_path = os.path.join(root, dir_)
                    targets += [abs_dir_path]

        # sign targets
        for target in targets:

            # Sign target with corresponding timestamp server
            for timestamp_server in self.timestamp_servers:
                print("Attempt for timestamp server: {}".format(timestamp_server))
                signtool_cmd = 'signtool sign /f C:\Cert\Dolby.pfx /t {}'.format(timestamp_server)
                ret_s = self._sign_target(target, signtool_cmd)

                if ret_s != 0:
                    print("ERROR: {} returned {} during signing of {}. Checking another timestamp server.".format(signtool_cmd, ret_s, target))
                else:
                    print("Successfully signed: {}".format(target))
                    break

            # Verify target
            signtool_verify_cmd = 'signtool verify /pa'
            ret_v = self._verify_target(target, signtool_verify_cmd)

            if ret_v != 0:
                raise IOError("ERROR: {} returned {} during verifycation of {}.".format(signtool_verify_cmd, ret_v, target))

        print("Done with {} targets.".format(len(targets)))


if __name__ == '__main__':
    getattr(Sign(os.environ["INPUT_SIGN_PATH"]),
            'sign_{}'.format(os.environ["INPUT_PLATFORM"])
           )()
