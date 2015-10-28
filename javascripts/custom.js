$(document).ready(function()
{
  var primesieve_version = '5.4.2';
  var primesieve_win64_binary = 'primesieve-5.4-win64.zip';
  var primesieve_win32_binary = 'primesieve-3.6-win32.zip';
  var primesieve_macosx_x64_binary = 'primesieve-5.4-macosx-x64.zip';
  var primesieve_linux_x64_binary = 'primesieve-5.4-linux-x64.tar.gz';
  var primesieve_linux_x86_binary = 'primesieve-3.6-linux-x86.tar.gz';
  var primesieve_os_binary = '';

  var download_url = 'http://dl.bintray.com/kimwalisch/primesieve/';
  var download_button_text = '.zip file';
  var is64bit = false;
  
  isUserAgent = function(s)
  {
    return navigator.userAgent.indexOf(s) > -1;
  }

  isPlatform = function(s)
  {
    return navigator.platform.indexOf(s) > -1;
  }

  isAppVersion = function(s)
  {
    return navigator.appVersion.indexOf(s) > -1;
  }

  // https://github.com/peterhurford/64or32
  if (isUserAgent('x86_64') ||
      isUserAgent('x86-64') ||
      isUserAgent('Win64') ||
      isUserAgent('x64;') ||
      isUserAgent('amd64') ||
      isUserAgent('AMD64') ||
      isUserAgent('WOW64') ||
      isUserAgent('x64_64') ||
      isPlatform('MacIntel') ||
      isPlatform('Linux x86_64'))
  {
    is64bit = true;
  }

  if (isAppVersion('Win'))
  {
    if (is64bit)
    {
      primesieve_os_binary = primesieve_win64_binary;
      download_button_text = 'Win64 .zip';
    }
    else
    {
      primesieve_os_binary = primesieve_win32_binary;
      download_button_text = 'Win32 .zip';
    }
  }
  else if (isAppVersion('Mac'))
  {
    if (is64bit)
    {
      primesieve_os_binary = primesieve_macosx_x64_binary;
      download_button_text = 'OS X .zip';
    }
  }
  else if (isAppVersion('Linux') ||
           isPlatform('Linux'))
  {
    if (is64bit)
    {
      primesieve_os_binary = primesieve_linux_x64_binary;
      download_button_text = 'Linux x64';
    }
    else if (isPlatform('i686'))
    {
      primesieve_os_binary = primesieve_linux_x86_binary;
      download_button_text = 'Linux x86';
    }
  }

  // fallback mode, offer source code for download
  if (primesieve_os_binary != '') {
    $('a.download_button').attr('href', download_url + primesieve_os_binary);
  }
  else
  {
    // use .tar.gz for all OSes except Windows
    var file_extension = '.tar.gz';
    if (isAppVersion('Win') != -1)
      file_extension = '.zip';

    download_button_text = file_extension + ' file';
    $('a.download_button').attr('href', download_url + 'primesieve-' + primesieve_version + file_extension);
  }

  $('a.download_button span').html(download_button_text);
  $('a.download_zip').attr('href', download_url + 'primesieve-' + primesieve_version + '.zip');
  $('a.download_targz').attr('href', download_url + 'primesieve-' + primesieve_version + '.tar.gz');
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
