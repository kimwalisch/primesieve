$(document).ready(function() {
	var primesieve_download_url = "http://dl.bintray.com/kimwalisch/primesieve/primesieve-";
	var primesieve_version = 5.0;
	$("a#download_zip").attr("href", primesieve_download_url + primesieve_version + ".zip");
	$("a#download_targz").attr("href", primesieve_download_url + primesieve_version + ".tar.gz");
});
