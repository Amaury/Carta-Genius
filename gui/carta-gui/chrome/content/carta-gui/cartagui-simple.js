/* **************** Constructor ************** */

function CartaGui() {
	this.start = CartaGui_Start;
	this.quit = CartaGui_Quit;
	this.newFile = CartaGui_NewFile;
	this.openFile = CartaGui_OpenFile;
	this.saveFile = CartaGui_SaveFile;
	this.generatePDF = CartaGui_GeneratePDF;
	this.setEditState = CartaGui_SetEditState;
	this.help = CartaGui_Help;
	this.about = CartaGui_About;
	this._enableInterface = _CartaGui_EnableInterface;
	this.wizNew = CartaGui_WizNew;
	this.wizCircle = CartaGui_WizCircle;
	this.wizBox = CartaGui_WizBox;
	this.wizLine = CartaGui_WizLine;
	this.wizBezier = CartaGui_WizBezier;
	this.wizImage = CartaGui_WizImage;
	this.wizGrid = CartaGui_WizGrid;
	this.wizHexagon = CartaGui_WizHexagon;
	this.wizText = CartaGui_WizText;
	this.wizVar = CartaGui_WizVar;
	this.insert = CartaGui_Insert;

	this._state = "empty";	// empty, edit, saved
	this._filePath = "";	// full path of the opened file
	this._destPath = "";	// full path of the PDF destination file
	this._exePath = "";	// full path of the Carta-Genius executable
	this._logPath = "";	// full path of the log file
	/*
	this._mainWindow
	this._textContent
	this._itemSave
	this._itemGenerate
	this._filePath
	*/
}

/* *************** Public Methods *************** */

function CartaGui_Insert(text, offset) {
	this._textContent.focus();
	var index1 = this._textContent.selectionStart;
	var index2 = this._textContent.selectionEnd;
	var res = this._textContent.value.substr(0, index1);
	res += text;
	var len = res.length;
	res += this._textContent.value.substr(index2);
	this._textContent.value = res;
	if (offset)
		this._textContent.setSelectionRange(offset, offset);
	else
		this._textContent.setSelectionRange(len, len);
}

function CartaGui_Start() {
	this._mainWindow = document.getElementById("main-window");
	this._textContent = document.getElementById("text-content");
	this._itemSave = document.getElementById("item-save");
	this._itemGenerate = document.getElementById("item-generate");
	this._itemsElements = new Array();
	this._itemCard = document.getElementById("item-card");
	this._itemCircle = document.getElementById("item-circle");
	this._itemBox = document.getElementById("item-box");
	this._itemLine =  document.getElementById("item-line");
	this._itemBezier = document.getElementById("item-bezier");
	this._itemImage = document.getElementById("item-image");
	this._itemPolygon = document.getElementById("item-polygon");
	this._itemGrid = document.getElementById("item-grid");
	this._itemHexagon = document.getElementById("item-hexagon");
	this._itemText = document.getElementById("item-text");
	this._itemVar = document.getElementById("item-var");
	this._itemTextDefine = document.getElementById("item-textdefine");
	this._itemTextArea = document.getElementById("item-textarea");
	this._itemDoc = document.getElementById("item-help");
	
	// read the configuration file
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath("c:\\winnt\\carta-gui.cfg");
	if (!file.exists())
		file.initWithPath("c:\\windows\\carta-gui.cfg");
	/*
	if (!file.exists())
		file.initWithPath("~/.carta-gui.cfg");
	*/
	if (!file.exists()) {
		alert("Impossible de trouver le fichier de configuration.\r\n" +
		      "Reportez-vous à la documentation.");
		window.close();
		return (false);
	}

	var data = "";
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, 1, 0, false);
	sstream.init(fstream);
	data += sstream.read(-1);
	sstream.close();
	fstream.close();
	data = data.replace("\r\n", "");
	data = data.replace("\n", "");

	// search the executable
	var file2 = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file2.initWithPath(data + "\\bin\\carta-genius.exe");
	if (file2.exists())
		this._exePath = data + "\\bin\\carta-genius.exe";
	else {
		alert("Impossible de trouver le programme Carta-Genius dans le répertoire\r\n" +
		      data + "\\bin\r\n\r\n" +
		      "Reportez-vous a la documentation.");
		window.close();
		return (false);
	}
	// set the log path
	this._logPath = data + "\\exec.log";
	// search the documentation
	file2.initWithPath(data + "\\doc\\carta-genius-fr.pdf");
	if (file2.exists()) {
		this._docPath = data + "\\doc\\carta-genius-fr.pdf";
		this._itemDoc.setAttribute("disabled", "false");
	}
}

function CartaGui_Quit() {
	if (this._state == "edit" &&
	    confirm("Sauvegarder le fichier en cours ?"))
		this.saveFile();
	window.close();
}

function CartaGui_NewFile() {
	if (this._state == "edit" &&
	    confirm("Sauvegarder le fichier en cours ?")) {
		this.saveFile();
	}
	this._textContent.value = "";
	
	this._filePath = "";
	this._destPath = "";
	this._state = "edit";
	this._enableInterface();
}

function CartaGui_OpenFile() {
	var nsIFilePicker = Components.interfaces.nsIFilePicker;
	var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
	fp.init(window, "Ouvrir un fichier", nsIFilePicker.modeOpen);
	fp.appendFilter("Fichiers Carta-Genius", "*.cgml; *.xml");
	fp.appendFilter("Tous", "*.*");

	var res = fp.show();
	if (res != nsIFilePicker.returnOK)
		return (false);

	var thefile = fp.file;
	if (thefile.exists() == false)
		return (false);
	
	var filename = thefile.leafName;
	var path = thefile.path;
	this._filePath = path;
	
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.QueryInterface(Components.interfaces.nsIFile);
	file.initWithPath(path);
	
	var data = "";
	var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
	var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"].createInstance(Components.interfaces.nsIScriptableInputStream);
	fstream.init(file, 1, 0, false);
	sstream.init(fstream);
	data += sstream.read(-1);
	sstream.close();
	fstream.close();
	
	this.newFile();
	this._filePath = path;
	this._textContent.value = data;
	this._state = "saved";
	this._destPath = "";
	this._enableInterface();
	
	window.title = "Carta-GUI | " + this._filePath;
	return (true);
}

function CartaGui_SaveFile() {
	if (!this._filePath.length) {
		var nsIFilePicker = Components.interfaces.nsIFilePicker;
		var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
		fp.init(window, "Enregistrement du fichier", nsIFilePicker.modeSave);
		fp.appendFilter("Fichiers Carta-Genius", "*.cgml; *.xml");
		fp.appendFilter("Tous", "*.*");

		var res = fp.show();
		if (res != nsIFilePicker.returnOK &&
		    res != nsIFilePicker.returnReplace)
			return (false);

		var thefile = fp.file;
		this._filePath = thefile.path;
	}
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.QueryInterface(Components.interfaces.nsIFile);
	file.initWithPath(this._filePath);
	if (!file.exists())
		file.create(0, 0777);
	
	var fstream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
	var sstream = Components.classes["@mozilla.org/binaryoutputstream;1"].createInstance(Components.interfaces.nsIBinaryOutputStream);
	fstream.init(file, 01102, 0777, false);
	sstream.setOutputStream(fstream);
	sstream.writeBytes(this._textContent.value, this._textContent.value.length);
	sstream.close();
	fstream.close();

	this._state = "saved";
	window.title = "Carta-GUI | " + this._filePath;
}

function CartaGui_GeneratePDF() {
	if (this._state == "edit")
		this.saveFile();
	if (!this._destPath.length) {
		var nsIFilePicker = Components.interfaces.nsIFilePicker;
		var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
		fp.init(window, "Choisissez un fichier PDF", nsIFilePicker.modeSave);
		fp.appendFilter("Fichiers PDF", "*.pdf");
		fp.appendFilter("Tous", "*.*");

		var res = fp.show();
		if (res != nsIFilePicker.returnOK &&
		    res != nsIFilePicker.returnReplace)
			return (false);

		this._destPath = fp.file.path;
	}

	// search the log file
	var logFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	logFile.initWithPath(this._logPath);
	if (logFile.exists())
		logFile.remove(0);

	// create an nsILocalFile for the executable
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(this._exePath);
	if (!file.exists()) {
		alert("Impossible de trouver le programme Carta-Genius\r\n" +
		      "'" + this._exePath + "'\r\n\r\n" +
		      "Reportez-vous a la documentation.");
		return (false);
	}
	
	// create an nsIProcess
	var process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
	process.init(file);
	
	// Run the process.
	var args = ["-i", this._filePath, "-o", this._destPath, "-L", this._logPath, "-q"];
	process.run(true, args, args.length);

	// open the log file if it exists
	var logFile2 = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	logFile2.initWithPath(this._logPath);

	if (process.exitValue && logFile2.exists() && logFile2.fileSize > 2) {
		alert("Echec lors de la création du fichier PDF.");
		logFile.launch();
		return (false);
	} else if (process.exitValue) {
		alert("Echec lors de la création du fichier PDF.\r\n" +
		      "Impossible de trouver le fichier de log.\r\n" +
		      "Le programme s'est terminé de manière anormale.");
		return (false);
	} else if (logFile2.exists() && logFile2.fileSize > 2)
		logFile.launch();

	// show the PDF file
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(this._destPath);
	if (file.exists())
		file.launch();
	return (true);
}

function CartaGui_SetEditState() {
	this._state = "edit";
	this._enableInterface();
}

function CartaGui_Help() {
	var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	file.initWithPath(this._docPath);
	if (file.exists())
		file.launch();
	else
		alert("Problème pour ouvrir '" + this._docPath);
}

function CartaGui_About() {
	alert("Pandocréon Carta-GUI\r\n" +
	      "Pandocréon Carta-Genius\r\n" +
	      "Copyright (c) 2005, Amaury Bouchard\r\n\r\n" +
	      "Ce programme est placé sous les termes de la\r\n" +
	      "Licence Publique Générale GNU (http://www.gnu.org).\r\n\r\n" +
	      "L'exécution de ce programme utilise l'interpréteur\r\n" +
	      "XULRunner du projet Mozilla (http://www.mozilla.org).\r\n\r\n" +
	      "Plus d'informations sur le projet Pandocréon :\r\n" +
	      "http://www.pandocreon.fr");
}

function CartaGui_WizNew() {
	window.open("chrome://carta-gui/content/wiz-new.xul", "wiz-new", "chrome,modal,resizable=no");
}

function CartaGui_WizCircle() {
	window.open("chrome://carta-gui/content/wiz-circle.xul", "wiz-circle", "chrome,modal,resizable=no");
}

function CartaGui_WizBox() {
	window.open("chrome://carta-gui/content/wiz-box.xul", "wiz-box", "chrome,modal,resizable=no");
}

function CartaGui_WizLine() {
	window.open("chrome://carta-gui/content/wiz-line.xul", "wiz-line", "chrome,modal,resizable=no");
}

function CartaGui_WizBezier() {
	window.open("chrome://carta-gui/content/wiz-bezier.xul", "wiz-bezier", "chrome,modal,resizable=no");
}

function CartaGui_WizImage() {
	window.open("chrome://carta-gui/content/wiz-image.xul", "wiz-image", "chrome,modal,resizable=no");
}

function CartaGui_WizGrid() {
	window.open("chrome://carta-gui/content/wiz-grid.xul", "wiz-grid", "chrome,modal,resizable=no");
}

function CartaGui_WizHexagon() {
	window.open("chrome://carta-gui/content/wiz-hexagon.xul", "wiz-hexagon", "chrome,modal,resizable=no");
}

function CartaGui_WizText() {
	window.open("chrome://carta-gui/content/wiz-text.xul", "wiz-text", "chrome,modal,resizable=no");
}

function CartaGui_WizVar() {
	window.open("chrome://carta-gui/content/wiz-var.xul", "wiz-var", "chrome,modal,resizable=no");
}

function _CartaGui_EnableInterface() {
	this._itemSave.setAttribute("disabled", "false");
	this._itemGenerate.setAttribute("disabled", "false");
	this._textContent.disabled = false;
	this._itemCircle.setAttribute("disabled", "false");
	this._itemBox.setAttribute("disabled", "false");
	this._itemLine.setAttribute("disabled", "false");
	this._itemBezier.setAttribute("disabled", "false");
	this._itemImage.setAttribute("disabled", "false");
	this._itemGrid.setAttribute("disabled", "false");
	this._itemHexagon.setAttribute("disabled", "false");
	this._itemText.setAttribute("disabled", "false");
	this._itemVar.setAttribute("disabled", "false");
}
