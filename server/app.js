const express = require("express");
const app = express();
var cors = require("cors");
const axios = require("axios");

app.use(cors());

axios.defaults.headers.get['Access-Control-Allow-Origin'] = '*';
const url = "http://192.168.0.129/?command="

app.get("/power", (req, res) => {
  let command = req.query.command;
  console.log(command);
  axios.get(url + command)
  .then((response) => {
    if(response.ok && response.status === 200)
      res.status(200);
    else
    res.send('Server error', 500);
  }, error => {
    console.log(error);
    res.send('Server error', 500);
  });
  res.status(200);

  axios.get(url, {
    headers: {
      "Access-Control-Allow-Origin": "*",
      Accept: "application/json",
    },
    crossdomain: true,
  })
  .then((response) => {
      res.send(response.data);
      res.send('OK', 200);
    }, (error) => {
      console.log(error);
      res.send('Server error', 500);
    }
  );
});

app.get("/get_data", (req, res) => {
    axios.get(url + "2", {
      headers: {
        Accept: "application/json",
      },
      crossdomain: true,
    })
    .then((response) => {
      if(response.ok && response.status === 200){
        res.send(response.data);
        res.send('OK', 200);
      } else
        res.send('Server error', 500);
    }, error => {
      console.log(error);
      res.send('Server error', 500);
    });
});

app.listen(8080, () => console.log(`Started server at http://localhost:8080!`));