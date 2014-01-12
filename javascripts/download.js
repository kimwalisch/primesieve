$(document).ready(function() {
	var primesieve_download_url = "http://dl.bintray.com/kimwalisch/primesieve/primesieve-";
	var primesieve_version = "5.0";
	$("a#download_zip").attr("href", primesieve_download_url + primesieve_version + ".zip");
	$("a#download_targz").attr("href", primesieve_download_url + primesieve_version + ".tar.gz");
});

(function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
})(window,document,'script','//www.google-analytics.com/analytics.js','ga');

ga('create', 'UA-47069346-1', 'primesieve.org');
ga('send', 'pageview');
