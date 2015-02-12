<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>unistd_2h</name>
    <member kind="function">
      <type>int</type>
      <name>gethostname</name>
      <anchorfile>http://pubs.opengroup.org/onlinepubs/009695399/functions/gethostname.html</anchorfile>
      <anchor></anchor>
      <arglist>(char *, size_t)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>getopt</name>
      <anchorfile>http://pubs.opengroup.org/onlinepubs/009695399/functions/getopt.html</anchorfile>
      <anchor></anchor>
      <arglist>(int argc, char * const argv[], const char *optstring)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>netdb_3h</name>
    <member kind="typedef">
      <type>struct hostent</type>
      <name>hostent</name>
      <anchorfile>http://pubs.opengroup.org/onlinepubs/009695399/basedefs/netdb.h.html</anchorfile>
      <anchor></anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>struct hostent *</type>
      <name>gethostbyname</name>
      <anchorfile>http://pubs.opengroup.org/onlinepubs/009695399/functions/gethostbyname.html</anchorfile>
      <anchor></anchor>
      <arglist>(const char *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>nagios_8h</name>
    <member kind="function">
      <type>int</type>
      <name>get_raw_command_line_r</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/nagios_8h.html</anchorfile>
      <anchor>ae70412efd5515a1dbce1400de4fe1d64</anchor>
      <arglist>(nagios_macros *, command *, char *, char **, int)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>macros_8h</name>
    <member kind="typedef">
      <type>struct nagios_macros</type>
      <name>nagios_macros</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/macros_8h.html</anchorfile>
      <anchor>structnagios__macros</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>grab_host_macros_r</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/macros_8h.html</anchorfile>
      <anchor>a224f5b0da1e1eb85089d20da495b521f</anchor>
      <arglist>(nagios_macros *, host *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>grab_service_macros_r</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/macros_8h.html</anchorfile>
      <anchor>ac9223042d23a261dc70a9704c1552843</anchor>
      <arglist>(nagios_macros *, service *)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>process_macros_r</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/macros_8h.html</anchorfile>
      <anchor>a81e80bf0f4be5046e3a78d1c1be465aa</anchor>
      <arglist>(nagios_macros *, char *, char **, int)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>objects_8h</name>
    <member kind="typedef">
      <type>struct host_struct</type>
      <name>host</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>structhost__struct</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct service_struct</type>
      <name>service</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>structservice__struct</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct command_struct</type>
      <name>command</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>structcommand__struct</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>struct customvariablesmember_struct</type>
      <name>customvariablesmember</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>structcustomvariablesmember__struct</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>host*</type>
      <name>find_host</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>a8670b990ce8f8e96965f359a3eadf3fc</anchor>
      <arglist>(char *)</arglist>
    </member>
    <member kind="function">
      <type>service*</type>
      <name>find_service</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>ae7077605975d6f0a0819b0da433056fe</anchor>
      <arglist>(char *, char *)</arglist>
    </member>
    <member kind="function">
      <type>command*</type>
      <name>find_command</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/objects_8h.html</anchorfile>
      <anchor>ad0d212bd3167204ce24a4b7781fe72ee</anchor>
      <arglist>(char *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>nebmodules_8h</name>
    <member kind="function">
      <type>int</type>
      <name>neb_set_module_info</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/nebmodules_8h.html</anchorfile>
      <anchor>a90b6c62b07181c054c8516b634ca7e39</anchor>
      <arglist>(void *, int, char *)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>nebstructs_8h</name>
    <member kind="typedef">
      <type>struct nebstruct_service_check_struct</type>
      <name>nebstruct_service_check_data</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/nebstructs_8h.html</anchorfile>
      <anchor>structnebstruct__service__check__struct</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>nebcallbacks_8h</name>
    <member kind="define">
      <type>#define</type>
      <name>NEBCALLBACK_SERVICE_CHECK_DATA</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/nebcallbacks_8h.html</anchorfile>
      <anchor>aa9d1e2d3f3a0fd148e8ad2f1edae4563</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>neb_register_callback</name>
      <anchorfile>http://sourcecodebrowser.com/nagios3/3.4.1/nebcallbacks_8h.html</anchorfile>
      <anchor>a571e606461d97fcb52df0dbb4d71c75a</anchor>
      <arglist>(int, void *, int, int(*)(int, void *))</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>curl_8h</name>
    <member kind="typedef">
      <type>void</type>
      <name>CURL</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>ace655e3633b6533591283994d6b5cdda</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>enum CURLcode</type>
      <name>CURLcode</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>af0691941698240652e0a391394217951</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>enum CURLoption</type>
      <name>CURLoption</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>a683f408ac299e0409f0c8b3f73c36610</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>CURLcode</type>
      <name>curl_global_init</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>a913e575dba100c0e9f9f6420a054d5ab</anchor>
      <arglist>(long)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>curl_global_cleanup</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>a503816e4c3eefb2db99f82d6cd1ec9da</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>const char *</type>
      <name>curl_easy_strerror</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/curl_8h.html</anchorfile>
      <anchor>a1feaa896e865ad01f32cc46f6f8c72f5</anchor>
      <arglist>(CURLcode)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>easy_8h</name>
    <member kind="function">
      <type>CURL *</type>
      <name>curl_easy_init</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/easy_8h.html</anchorfile>
      <anchor>ac6359c6087e29eb8876d92bf8c7c10d7</anchor>
      <arglist>(void)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>curl_easy_cleanup</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/easy_8h.html</anchorfile>
      <anchor>a7acd7812b672df9e378050fab7c732aa</anchor>
      <arglist>(CURL *)</arglist>
    </member>
    <member kind="function">
      <type>CURLcode</type>
      <name>curl_easy_setopt</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/easy_8h.html</anchorfile>
      <anchor>a7b171739b7bb728b3b58b2e6ec454aa0</anchor>
      <arglist>(CURL *, CURLoption, ...)</arglist>
    </member>
    <member kind="function">
      <type>CURLcode</type>
      <name>curl_easy_perform</name>
      <anchorfile>http://fossies.org/dox/curl-7.40.0/easy_8h.html</anchorfile>
      <anchor>a8126b918a6e44facb451ce6c78658506</anchor>
      <arglist>(CURL *)</arglist>
    </member>
  </compound>
</tagfile>
