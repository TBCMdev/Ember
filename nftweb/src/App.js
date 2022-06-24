import React from 'react';
import './App.css';

import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';


function App() {
  return (
    <div className="App">
      <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css"></link>
      <div className='Main-Container'>
        <div className="title-bar">
          <ul>
            <li><a href="/">WEBSITE TITLE</a></li>
            <FontAwesomeIcon icon= 'fa-brands fa-discord' className='social' size='xl' color='white'></FontAwesomeIcon>
          </ul>
        </div>
      </div>
      <header className="App-header">
        WEBSITE TITLE
      </header>
    </div>
  );
}

export default App;
