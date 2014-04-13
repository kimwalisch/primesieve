$(document).ready(function() {
	// Workaround for Safari and Chrome CSS alignment bug (2014).
	$("h2").hide().fadeIn('fast');

	var primesieve_version = "5.2";
	var download_url = "http://dl.bintray.com/kimwalisch/primesieve/primesieve-";
	$("a.download_zip").attr("href", download_url + primesieve_version + ".zip");
	$("a.download_targz").attr("href", download_url + primesieve_version + ".tar.gz");
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
