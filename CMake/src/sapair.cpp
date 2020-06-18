#include "sapair.h"
#include "common.h"

SApair::SApair(asio::ip::tcp::socket* tcpSocket)
{
	auto remoteEp = tcpSocket->remote_endpoint();
	m_ipAddr = remoteEp.address();
	m_portNumber = remoteEp.port();

	if (m_ipAddr.is_v4())
	{
		m_saPairStr += STR_V4;
		auto _ipAddr4 = m_ipAddr.to_v4();
		m_saPairStr += " " + _ipAddr4.to_string();
	}
	else
	{
		m_saPairStr += STR_V6;
		auto _ipAddr6 = m_ipAddr.to_v6();
		if (_ipAddr6.is_v4_mapped())
			m_saPairStr += " " + _ipAddr6.to_v4().to_string();
		else
			m_saPairStr += " " + _ipAddr6.to_string();
	}
	m_saPairStr += " " + std::to_string(m_portNumber);
}

std::string SApair::toString() const
{
	return m_saPairStr;
}

bool SApair::operator==(const SApair& saPair) const
{
	if (m_ipAddr == saPair.m_ipAddr && m_portNumber == saPair.m_portNumber)
		return true;
	else
		return false;
}
