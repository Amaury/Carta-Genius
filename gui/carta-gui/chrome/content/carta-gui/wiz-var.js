function wizInit() {
	edit_name = document.getElementById("edit-name");
	edit_value = document.getElementById("edit-value");
	check_precision = document.getElementById("check-precision");
	menulist_precision = document.getElementById("menulist-precision");
	label_precision = document.getElementById("label-precision");
	value_precision = 0;
	
	edit_name.focus();
}

function checkPrecision() {
	if (check_precision.checked) {
		menulist_precision.disabled = false;
		label_precision.disabled = false;
	} else {
		menulist_precision.disabled = true;
		label_precision.disabled = true;
	}
}

function wizValid() {
	if (!edit_name.value.length) {
		alert("Mauvaise Nom");
		return;
	} else if (!edit_value.value.length) {
		alert("Mauvaise valeur");
		return;
	}
	
	var str = "<var name=\"" + xmlEncode(edit_name.value) + "\"";
	if (check_precision.checked)
		str += " precision=\"" + value_precision + "\"";
	str += ">" + xmlEncode(edit_value.value) + "</var>";
	window.opener.carta.insert(str, 0);
	window.close();
}
