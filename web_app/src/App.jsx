import { useEffect, useState } from 'react';

import logo from './logo.svg';
import './App.css';

function App() {
  const [data, setData] = useState("");

useEffect(() => {
  // GET request using fetch inside useEffect React hook
  fetch('http://192.168.0.129:80/?command=1')
      .then(response => setData(response))
      .then(console.log(data));

// empty dependency array means this effect will only run once (like componentDidMount in classes)
}, [data]);



  return (
    <div className="App">
      <header className="App-header">
        <img src={logo} className="App-logo" alt="logo" />
        <p>
          Edit <code>src/App.js</code> and save to reload.
        </p>
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
