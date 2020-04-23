// Modules /////////////////////////////////////////////////////////////////////////////////////////////
const express = require('express');
const ejs = require('ejs');
const app = express();
const {spawn} = require('child_process');
var bodyParser = require('body-parser');

app.set('view engine', 'ejs');

app.use( express.static( "public" ) );
app.use( bodyParser.json() ); 
app.use(bodyParser.urlencoded({ extended: false }));

app.get('/', (req, res) => res.render('index'));

function getRandomInt(max) {
	return Math.floor(Math.random() * Math.floor(max));
}

app.post('/submit', (req, res) => {
	var msg = req.body.msg;
	var dataToSend;
	// spawn new child process to call the python script
	const python = spawn('python', ['encode.py', "images/img" + (getRandomInt(100)+1).toString() + ".jpg", msg]);
	// collect data from script
	python.stdout.on('data', function (data) {
		console.log('Pipe data from python script ...');
		dataToSend = data.toString();
	});
	// in close event we are sure that stream from child process is closed
	python.on('close', (code) => {
		console.log(`child process close all stdio with code ${code}`);
		// send data to browser
		res.send(dataToSend)
	});
});


const port = 2222;
app.listen(port, () => console.log('Server started on port ' + port));




