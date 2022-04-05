// index.js
const express = require('express');
const basicAuth = require('express-basic-auth')
const bodyParser = require('body-parser')
const app = express();

var jsonParser = bodyParser.json();
var urlencodedParser = bodyParser.urlencoded({ extended: false })

/* app.use(basicAuth({
    users: {
        'neo': 'matrix',
        'adam': 'password1234',
        'eve': 'asdfghjkl',
    }
})) */

app.post('/post-test',jsonParser, (req, res) => {
    values = req.body;
    console.log(req.body);
    res.sendStatus(200);
});



app.listen(8080, () => console.log(`Started server at http://localhost:8080!`));