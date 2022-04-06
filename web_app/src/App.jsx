import { useEffect, useState } from 'react';
import axios from 'axios';

import logo from './logo.svg';
import './App.css';

function App() {
  const [data, setData] = useState("");

  const url = "http://localhost:8080";
  axios.defaults.headers.get['Access-Control-Allow-Origin'] = '*';

  const power = (command) => {
    axios.get(url + "/power?command=" + command)
    .then(response => {
      if(response.ok)
        alert("Turning on the light");
    }, error => {
      console.log(error);
    });
  };

  const fetchData = () => {
    axios.get(url + "/get_data", {
      headers: {
        Accept: "application/json",
      },
      crossdomain: true,
    })
    .then(response => {
      console.log(response.data);
    }, error => {
      console.log(error);
    });
  };

  return (
    <div className="App">
      <header className="App-header">
        <img src={logo} className="App-logo" alt="logo" />
        <p>
          Edit <code>src/App.js</code> and save to reload.
        </p>
        <button className="btn bg-white"onClick={() => fetchData()}>Get data</button>
        <button onClick={() => power('1')}>Power on/off</button>
        <button onClick={() => power('3')}>Reset</button>

        <a
          className="App-link"
          href="https://reactjs.org"
          target="_blank"
          rel="noopener noreferrer"
        >
          Learn React
        </a>
      </header>
    </div>
  );
}

export default App;
