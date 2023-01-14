import { useEffect, useState } from 'react';
import axios from 'axios';

import logo from './logo.svg';
import './App.css';
import "@material-tailwind/react/tailwind.css";

import Notification from './components/Notification/Notification';

function App() {
  const [data, setData] = useState("");
  const [showModal, setShowModal] = useState(false);

  let type = "based";
  let message = "";

  const url = "http://localhost:8080";
  axios.defaults.headers.get['Access-Control-Allow-Origin'] = '*';

  const power = (command) => {
    console.log("O");
    axios.get(url + "/power?command=" + command)
    .then(response => {
      if(response.ok){
        setShowModal(true);
        console.log("OOO");
      }
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
      if(response.status === 200)
        console.log(response.data);
    }, error => {
      console.log(error);
    });
  };

  return (
    <div className="App h-full flex flex-col bg-neutral-100 dark:bg-black text-black dark:text-white">
        <button className="button dark:text-white" onClick={() => fetchData()}>Get data</button>
        <button className="button dark:text-white" onClick={() => power('1')}>Power on/off</button>
        <button className="button dark:text-white" onClick={() => power('3')}>Reset</button>
        <p className="block text-white">Temp: 24°C | Humidity: 41%</p>
    </div>
  );
}

export default App;
