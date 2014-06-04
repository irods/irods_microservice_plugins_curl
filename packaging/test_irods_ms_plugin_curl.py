import sys
if (sys.version_info >= (2,7)):
    import unittest
else:
    import unittest2 as unittest
import pydevtest_common as c
import pydevtest_sessions as s


class Test_MS_Plugin_CURL(unittest.TestCase):
    rules_dir = c.get_irods_top_level_dir() + "/iRODS/clients/icommands/test/rules3.0/"
    dest_obj="/tempZone/home/public/ferrari.art" # will need to be passed to curlGetObj.r
    
    def setUp(self):
        s.admin_up()
        
    def tearDown(self):
        s.adminsession.runCmd('irm', ['-f', self.dest_obj])
        s.admin_down()
        
    def test_curl_get_obj(self):
        rule_file = self.rules_dir + "curlGetObj.r"
        print "-- running "+rule_file
        c.assertiCmd(s.adminsession,"irule -vF "+rule_file, "LIST", "completed successfully")
        
    def test_curl_get_str(self):
        rule_file = self.rules_dir + "curlGetStr.r"
        print "-- running "+rule_file
        c.assertiCmd(s.adminsession,"irule -vF "+rule_file, "LIST", "completed successfully")
        
    def test_curl_post(self):
        rule_file = self.rules_dir + "curlPost.r"
        print "-- running "+rule_file
        
        # will have to dynamically pass form_data to the rule once that's fixed
        form_data = "Sent from iRODS"
        c.assertiCmd(s.adminsession,"irule -F "+rule_file, "LIST", form_data)