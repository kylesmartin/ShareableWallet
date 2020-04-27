// Modules /////////////////////////////////////////////////////////////////////////////////////////////
const express = require('express');
const app = express();
const multer = require('multer');
const ejs = require('ejs');
const path = require('path');
const {spawn} = require('child_process');
var http = require('http').createServer(app);
var bodyParser = require('body-parser');

app.use( bodyParser.json() ); 
app.use(bodyParser.urlencoded({ extended: false }));

var steg = "No image uploaded";

// Set storage engine
const storage = multer.diskStorage({
    destination: "./public/uploads/",
    filename: function (req, file, cb) {
        var img = file.fieldname + "-" + Date.now() + path.extname(file.originalname);
        cb(null, img);
        
        var dataToSend;
        // spawn new child process to call the python script
        const python = spawn('python', ['decode.py', "public/uploads/"+img]);
        // collect data from script
        python.stdout.on('data', function (data) {
            console.log('Pipe data from python script ...');
            dataToSend = data.toString();
        });
        // in close event we are sure that stream from child process is closed
        python.on('close', (code) => {
            console.log(`child process close all stdio with code ${code}`);
            steg = dataToSend;
        });
    }
})

// Uploading
const upload = multer({
    storage: storage,
    fileFilter: function (req, file, cb) {
        checkFileType(file, cb);
    }
}).single("myImage");

// Check File Type
function checkFileType(file, cb) {
    // Allowed ext
    const filetypes = /jpeg|jpg|png/;
    // Check ext
    const extname = filetypes.test(path.extname(file.originalname).toLowerCase());
    // Check mime
    const mimetype = filetypes.test(file.mimetype);
    if (mimetype && extname) {
        return cb(null, true);
    } else {
        cb('Error: Images Only');
    }
}

app.set('view engine', 'ejs');
app.use(express.static('./public'));

app.get('/', (req, res) => res.render('index'));

app.post('/upload', (req, res) => {
    upload(req, res, (err) => {
        if (err) {
            res.render('index', {
                msg: err
            })
        } else {
            if (req.file == undefined) {
                res.render('index', {
                    msg: 'Error: No File Selected!'
                });
            } else {
                res.render('index', {
                    msg: 'File Uploaded!',
                    file: `uploads/${req.file.filename}`
                });
            }
        }
    });
});

app.get('/getmessage', (req, res) => {
    res.send(steg)
});

// Listening on port 3000
const port = 3000;
http.listen(port, function() {
  console.log('listening on *:3000');
});

/*
const port = 3000;
app.listen(port, () => console.log('Server started on port ' + port));
*/