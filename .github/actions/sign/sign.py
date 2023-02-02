import os
import sys
import base64


class Sign():
    def __init__(self, root_app):
        self.timestamp_servers = ['http://timestamp.comodoca.com/authenticode',
                                  'http://timestamp.verisign.com/scripts/timestamp.dll',
                                  'http://timestamp.globalsign.com/scripts/timstamp.dll',
                                  'http://timestamp.globalsign.com/tsa/r6advanced1',
                                  'http://www.startssl.com/timestamp']
        self.root_app = root_app
        self.file_ext_win = ['.dll', '.exe']
        self.dirs_win = []
        self.file_ext_darwin = ['.dylib']
        self.dir_darwin = ".app"

    def _sign_target(self, target, sign_cmd):

        print(f"Sign {target}")
        ret = os.system(f"{sign_cmd} \"{target}\"")

        return ret

    def _verify_target(self, target, verify_cmd):
        print(f"Verify {target}")
        ret = os.system(f"{verify_cmd} \"{target}\"")

        return ret

    def sign_macos(self):

        # Sign libs
        targets = list()

        # Define codesign cmd
        codesign_verify_cmd = "codesign --verify --deep --verbose=4"
        codesign_cmd = "codesign --force --strict --timestamp --sign 'Developer ID Application: VOXEET INC. (B55NRA8BRW)'"

        # collect targets
        for root, dirs, files in os.walk(self.root_app):
            for file_ in files:
                if os.path.splitext(file_)[-1] in self.file_ext_darwin:
                    targets += [os.path.join(root, file_)]

        targets_sorted = sorted(targets, key=lambda x: x.count(os.sep), reverse=True)

        for target in targets:
            ret_s = self._sign_target(target, codesign_cmd)

            if ret_s != 0:
                print(f"ERROR: {codesign_cmd} returned {ret_s} during signing {target}.")

            ret_v = self._verify_target(target, codesign_verify_cmd)

            if ret_v !=0:
                raise IOError(f"ERROR: {codesign_verify_cmd} returned {ret_v} during verification of {target}")

        print(f"{len(targets)} target(s) signed")

    def sign_windows(self):
        targets = list()
        passwd = os.environ["WINDOWS_CERTIFICATE_PASSWORD"]

        # signtool from Windows Kits 10 was added manually to env path on the Windows machine

        # collect targets
        for root, dirs, files in os.walk(self.root_app):
            for file_ in files:
                if os.path.splitext(file_)[-1] in self.file_ext_win:
                    abs_file_path = os.path.join(root, file_)
                    targets += [abs_file_path]

        # sign targets
        for target in targets:

            # Sign target with corresponding timestamp server
            for timestamp_server in self.timestamp_servers:
                print(f"Attempt for timestamp server: {timestamp_server}")
                signtool_cmd = f"signtool sign /fd SHA256 /f C:\Cert\Dolby.pfx /p {passwd} /t {timestamp_server}"
                ret_s = self._sign_target(target, signtool_cmd)

                if ret_s != 0:
                    print(f"ERROR: {signtool_cmd} returned {ret_s} during signing {target}. Checking another timestamp server.")
                else:
                    print(f"Signed: {target}")
                    break

            # Verify target
            signtool_verify_cmd = 'signtool verify /pa'
            ret_v = self._verify_target(target, signtool_verify_cmd)

            if ret_v != 0:
                raise IOError(f"ERROR: {signtool_verify_cmd} returned {ret_v} during verification of {target}.")

        print(f"{len(targets)} target(s) signed")


if __name__ == '__main__':
    getattr(Sign(os.environ["INPUT_SIGN_PATH"]),
            'sign_{}'.format(os.environ["INPUT_PLATFORM"])
           )()
