$(document).ready(function()
{
  // Update font sizes

  if (navigator.appVersion.indexOf("Mac") != -1 || navigator.appVersion.indexOf("iOS") != -1)
  {
    $('.gist .blob-code-inner').css('font-size', '1.2em');
    $('.terminal_body').css('font-size', '0.85em');
    $('.terminal_body').css('line-height', '1.4em');
  }
  else
  {
    $('.gist .blob-code-inner').css('font-size', '1.3em');
  }

  // Update download links

  var primesieve_version = '6.3';
  var primesieve_win64_binary = 'v6.2/primesieve-6.2-win64.zip';
  var primesieve_win32_binary = 'v3.6/primesieve-3.6-win32.zip';
  var primesieve_macosx_x64_binary = 'v5.5.0/primesieve-5.5.0-macOS-x64.zip';
  var primesieve_linux_x64_binary = 'v6.2/primesieve-6.2-linux-x64.tar.gz';
  var primesieve_linux_x86_binary = 'v3.6/primesieve-3.6-linux-x86.tar.gz';
  var primesieve_os_binary = '';

  var download_url = 'https://github.com/kimwalisch/primesieve/releases/download/';
  var download_button_text = 'Source .zip';
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
      download_button_text = 'macOS .zip';
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

  if (primesieve_os_binary != '')
  {
    $('a.download_button').attr('href', download_url + primesieve_os_binary);
    $('a.download_button span').html(download_button_text);
  }
  else
  {
    // fallback mode, offer source code for download
    $('a.download_button').attr('href', 'https://github.com/kimwalisch/primesieve/archive/v' + primesieve_version + '.zip');
    $('a.download_button span').html(download_button_text);
  }
});
