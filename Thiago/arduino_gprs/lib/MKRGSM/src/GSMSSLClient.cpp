/*
  This file is part of the MKR GSM library.
  Copyright (C) 2017  Arduino AG (http://www.arduino.cc/)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "utility/GSMRootCerts.h"

#include "Modem.h"

#include "GSMSSLClient.h"

enum {
  SSL_CLIENT_STATE_LOAD_ROOT_CERT,
  SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE,
  SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE
};

bool GSMSSLClient::_rootCertsLoaded = false;

GSMSSLClient::GSMSSLClient(bool synch) :
  GSMClient(synch)
{
}

GSMSSLClient::~GSMSSLClient()
{
}

int GSMSSLClient::ready()
{
  if (_rootCertsLoaded) {
    // root certs loaded already, continue to regular GSMClient
    return GSMClient::ready();
  }

  int ready = MODEM.ready();

  if (ready == 0) {
    // a command is still running
    return 0;
  }

  switch (_state) {
    case SSL_CLIENT_STATE_LOAD_ROOT_CERT: {
      if (GSM_ROOT_CERTS[_certIndex].size) {
        // load the next root cert
        MODEM.sendf("AT+USECMNG=0,0,\"%s\",%d", GSM_ROOT_CERTS[_certIndex].name, GSM_ROOT_CERTS[_certIndex].size);
        if (MODEM.waitForPrompt() != 1) {
          // failure
          ready = -1;
        } else {
          // send the cert contents
          MODEM.write(GSM_ROOT_CERTS[_certIndex].data, GSM_ROOT_CERTS[_certIndex].size);

          _state = SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE;
          ready = 0;
        }
      } else {
        // remove the next root cert name
        MODEM.sendf("AT+USECMNG=2,0,\"%s\"", GSM_ROOT_CERTS[_certIndex].name);

        _state = SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE;
        ready = 0;
      }
      break;
    }

    case SSL_CLIENT_STATE_WAIT_LOAD_ROOT_CERT_RESPONSE: {
      if (ready > 1) {
        // error
      } else {
        _certIndex++;

        if (_certIndex == GSM_NUM_ROOT_CERTS) {
          // all certs loaded
          _rootCertsLoaded = true;
        } else {
          // load next
          _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;
        }

        ready = 0;
      }
      break;
    }

    case SSL_CLIENT_STATE_WAIT_DELETE_ROOT_CERT_RESPONSE: {
      // ignore ready response, root cert might not exist
      _certIndex++;

      if (_certIndex == GSM_NUM_ROOT_CERTS) {
        // all certs loaded
        _rootCertsLoaded = true;
      } else {
        // load next
        _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;
      }

      ready = 0;
      break;
    }
  }

  return ready;
}

int GSMSSLClient::connect(IPAddress ip, uint16_t port)
{
  _certIndex = 0;
  _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;

  return connectSSL(ip, port);
}

int GSMSSLClient::connect(const char* host, uint16_t port)
{
  _certIndex = 0;
  _state = SSL_CLIENT_STATE_LOAD_ROOT_CERT;

  return connectSSL(host, port);
}
