'use strict';

$(function () {
	let url_href = window.location.href;
	let url_dir = url_href.substring(0, url_href.lastIndexOf('/'));
	let url_pardir = url_dir.substring(0, url_dir.lastIndexOf('/'));
    $.get(url_pardir + '/version_selector.html', function (data) {
        $('#projectnumber').html(data);

        document.getElementById('version_selector').addEventListener('change', function () {
            let selected_version = this.value;
            window.location.href = url_pardir + '/' + selected_version + '/index.html';
        });
		
		let url_parts = window.location.pathname.split('/');
        let current_version = url_parts[url_parts.length - 2];
		console.log(current_version)
        $('#version_selector').val(current_version);
    });
});
