from __future__ import print_function

import optparse
import os
import shutil
import glob

import irods_python_ci_utilities

def install_test_prerequisites():
    irods_python_ci_utilities.subprocess_get_output(['sudo', 'pip', 'install', 'boto3', '--upgrade'], check_rc=True)
    irods_python_ci_utilities.subprocess_get_output(['sudo', '-EH', 'pip', 'install', 'unittest-xml-reporting==1.14.0'])

def main():
    parser = optparse.OptionParser()
    parser.add_option('--output_root_directory')
    parser.add_option('--built_packages_root_directory')
    parser.add_option('--test', metavar='dotted name')
    parser.add_option('--skip-setup', action='store_false', dest='do_setup', default=True)
    options, _ = parser.parse_args()

    built_packages_root_directory = options.built_packages_root_directory
    package_suffix = irods_python_ci_utilities.get_package_suffix()
    os_specific_directory = irods_python_ci_utilities.append_os_specific_directory(built_packages_root_directory)

    if options.do_setup:
        irods_python_ci_utilities.install_os_packages_from_files(
            glob.glob(os.path.join(os_specific_directory,
                      'irods-microservice-plugins-curl*.{}'.format(package_suffix))
            )
        )

        install_test_prerequisites()

    test = options.test or 'test_irods_microservice_plugins_curl'

    try:
        test_output_file = 'log/test_output.log'
        irods_python_ci_utilities.subprocess_get_output(['sudo', 'su', '-', 'irods', '-c',
            'python2 scripts/run_tests.py --xml_output --run_s {} 2>&1 | tee {}; exit $PIPESTATUS'.format(
            test, test_output_file)], check_rc=True)
    finally:
        output_root_directory = options.output_root_directory
        if output_root_directory:
            irods_python_ci_utilities.gather_files_satisfying_predicate('/var/lib/irods/log', output_root_directory, lambda x: True)
            shutil.copy('/var/lib/irods/log/test_output.log', output_root_directory)
            #shutil.copytree('/var/lib/irods/test-reports', os.path.join(output_root_directory, 'test-reports'))


if __name__ == '__main__':
    main()
