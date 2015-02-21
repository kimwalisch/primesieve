$(document).ready(function() {
	// Workaround for Safari and Chrome CSS alignment bug (2014).
	$("h2").hide().fadeIn('fast');
  
  var download_url = "http://dl.bintray.com/kimwalisch/primesieve/";
  var primesieve_version = "";
  var download_button_text = ".zip file";
  var is64bit = false;
  
  is64bitOS = function(s)
  {
    return window.navigator.userAgent.indexOf(s) > -1;
  }

  // https://github.com/peterhurford/64or32
  if (is64bitOS('x86_64') ||
      is64bitOS('x86-64') ||
      is64bitOS('Win64') ||
      is64bitOS('x64;') ||
      is64bitOS('amd64') ||
      is64bitOS('AMD64') ||
      is64bitOS('WOW64') ||
      is64bitOS('x64_64') ||
      window.navigator.platform === 'MacIntel' ||
      window.navigator.platform === 'Linux x86_64')
  {
    is64bit = true;
  }

  if (navigator.appVersion.indexOf("Win") != -1)
  {
    if (is64bit)
    {
      primesieve_version = "primesieve-5.4-win64.zip";
      download_button_text = "Win64 .zip";
    }
    else
    {
      primesieve_version = "primesieve-3.6-win32.zip";
      download_button_text = "Win32 .zip";
    }
  }
  else if (navigator.appVersion.indexOf("Mac") != -1)
  {
    if (is64bit)
    {
      primesieve_version = "primesieve-5.4-macosx-x64.zip";
      download_button_text = "OS X .zip";
    }
  }
  else if (navigator.appVersion.indexOf("Linux") !=-1)
  {
    if (is64bit)
    {
      primesieve_version = "primesieve-5.4-linux-x64.tar.gz";
      download_button_text = "Linux-x64";
    }
    else if (window.navigator.platform == "Linux i686" ||
             window.navigator.platform == "Linux i686 X11" ||
             window.navigator.platform == "Linux x86")
    {
      primesieve_version = "primesieve-3.6-linux-x86.tar.gz";
      download_button_text = "Linux-x86";
    }
  }

  // fallback mode, offer source code for download
  if (primesieve_version != "") {
    $("a.download_zip").attr("href", download_url + primesieve_version);
  }
  else
  {
    // use .tar.gz for all OSes except Windows
    var download_file_extension = ".zip";
    if (navigator.appVersion.indexOf("Win") == -1)
      download_file_extension = ".tar.gz";

    download_button_text = download_file_extension + " file";
    $("a.download_zip").attr("href", download_url + "primesieve-5.4.1" + download_file_extension);
  }

  // update file extension in download button
  $("a.download_zip span").html(download_button_text);
});

if(!Modernizr.svg) {
  $('img[src$="svg"]').attr('src', function() {
    return $(this).attr('src').replace('.svg', '.png');
  });
}

(function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
})(window,document,'script','//www.google-analytics.com/analytics.js','ga');

ga('create', 'UA-47069346-1', 'primesieve.org');
ga('send', 'pageview');
