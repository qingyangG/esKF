# CMake generated Testfile for 
# Source directory: /home/qingyang/workspace/src/nmea_navsat_driver
# Build directory: /home/qingyang/workspace/build/nmea_navsat_driver
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(_ctest_nmea_navsat_driver_venv_check_nmea_navsat_driver-requirements "/home/qingyang/workspace/build/catkin_generated/env_cached.sh" "/usr/bin/python2" "/opt/ros/melodic/share/catkin/cmake/test/run_tests.py" "/home/qingyang/workspace/build/test_results/nmea_navsat_driver/venv_check-nmea_navsat_driver-requirements.xml" "--working-dir" "/home/qingyang/workspace/build" "--return-code" "/home/qingyang/workspace/build/catkin_generated/env_cached.sh rosrun catkin_virtualenv venv_check venv --requirements /home/qingyang/workspace/src/nmea_navsat_driver/test/requirements.txt         --extra-pip-args \"\\\"-qq --retries 10 --timeout 30\\\"\"         --xunit-output /home/qingyang/workspace/build/test_results/nmea_navsat_driver/venv_check-nmea_navsat_driver-requirements.xml")
add_test(_ctest_nmea_navsat_driver_nosetests_test.test_driver.py "/home/qingyang/workspace/build/catkin_generated/env_cached.sh" "/usr/bin/python2" "/opt/ros/melodic/share/catkin/cmake/test/run_tests.py" "/home/qingyang/workspace/build/test_results/nmea_navsat_driver/nosetests-test.test_driver.py.xml" "--return-code" "\"/usr/bin/cmake\" -E make_directory /home/qingyang/workspace/build/test_results/nmea_navsat_driver" "/home/qingyang/workspace/devel/share/nmea_navsat_driver/venv/bin/python -m nose -P --process-timeout=60 /home/qingyang/workspace/src/nmea_navsat_driver/test/test_driver.py --with-xunit --xunit-file=/home/qingyang/workspace/build/test_results/nmea_navsat_driver/nosetests-test.test_driver.py.xml")
add_test(_ctest_nmea_navsat_driver_roslint_package "/home/qingyang/workspace/build/catkin_generated/env_cached.sh" "/usr/bin/python2" "/opt/ros/melodic/share/catkin/cmake/test/run_tests.py" "/home/qingyang/workspace/build/test_results/nmea_navsat_driver/roslint-nmea_navsat_driver.xml" "--working-dir" "/home/qingyang/workspace/build/nmea_navsat_driver" "--return-code" "/opt/ros/melodic/share/roslint/cmake/../../../lib/roslint/test_wrapper /home/qingyang/workspace/build/test_results/nmea_navsat_driver/roslint-nmea_navsat_driver.xml make roslint_nmea_navsat_driver")