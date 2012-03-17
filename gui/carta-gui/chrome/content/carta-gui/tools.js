function xmlEncode(str) {
	str = str.replace(/&/g, "&amp;");
	str = str.replace(/\"/g, "&quot;");
	str = str.replace(/</g, "&lt;");
	str = str.replace(/>/g, "&gt;");
	return (str);
}
