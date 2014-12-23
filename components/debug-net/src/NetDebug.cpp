/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/* Debugging overlay for the 'net' component. */

#include "StdInc.h"
#include "NetLibrary.h"
#include "FontRenderer.h"
#include "DrawCommands.h"
#include "Screen.h"
#include <mmsystem.h>

const int g_netOverlayOffsetX = -30;
const int g_netOverlayOffsetY = -30;
const int g_netOverlayWidth = 400;
const int g_netOverlayHeight = 300;

const int g_netOverlaySampleSize = 200; // milliseconds per sample frame
const int g_netOverlaySampleCount = 150;

class NetOverlayMetricSink : public INetMetricSink
{
public:
	NetOverlayMetricSink();

	virtual void OnIncomingPacket(const NetPacketMetrics& packetMetrics);

	virtual void OnOutgoingPacket(const NetPacketMetrics& packetMetrics);

	virtual void OnPingResult(int msec);

private:
	int m_ping;

	int m_lastInPackets;
	int m_lastOutPackets;

	int m_lastInBytes;
	int m_lastOutBytes;

	int m_inPackets;
	int m_outPackets;

	int m_inBytes;
	int m_outBytes;

	NetPacketMetrics m_metrics[g_netOverlaySampleCount + 1];

	uint32_t m_lastUpdatePerSec;

	uint32_t m_lastUpdatePerSample;

private:
	inline int GetOverlayLeft()
	{
		if (g_netOverlayOffsetX < 0)
		{
			return GetScreenResolutionX() + g_netOverlayOffsetX - g_netOverlayWidth;
		}
		else
		{
			return g_netOverlayOffsetX;
		}
	}

	inline int GetOverlayTop()
	{
		if (g_netOverlayOffsetY < 0)
		{
			return GetScreenResolutionY() + g_netOverlayOffsetY - g_netOverlayHeight;
		}
		else
		{
			return g_netOverlayOffsetY;
		}
	}

	CRGBA GetColorIndex(int i);

	void UpdateMetrics();

	void DrawBaseMetrics();

	void DrawGraph();
};

NetOverlayMetricSink::NetOverlayMetricSink()
	: m_ping(0), m_lastInBytes(0), m_lastInPackets(0), m_lastOutBytes(0), m_lastOutPackets(0),
	  m_lastUpdatePerSample(0), m_lastUpdatePerSec(0),
	  m_inBytes(0), m_inPackets(0), m_outBytes(0), m_outPackets(0)
{
	OnPostFrontendRender.Connect([=] ()
	{
		// update metrics
		UpdateMetrics();

		// draw the base metrics
		DrawBaseMetrics();

		// draw the graph
		DrawGraph();
	}, 50);
}

void NetOverlayMetricSink::OnIncomingPacket(const NetPacketMetrics& packetMetrics)
{
	m_metrics[g_netOverlaySampleCount] = m_metrics[g_netOverlaySampleCount] + packetMetrics;

	m_inPackets++;
	m_inBytes += packetMetrics.GetTotalSize();
}

void NetOverlayMetricSink::OnOutgoingPacket(const NetPacketMetrics& packetMetrics)
{
	m_outPackets++;
	m_outBytes += packetMetrics.GetTotalSize();
}

void NetOverlayMetricSink::OnPingResult(int msec)
{
	m_ping = msec;
}

void NetOverlayMetricSink::UpdateMetrics()
{
	uint32_t time = timeGetTime();

	if ((time - m_lastUpdatePerSample) > g_netOverlaySampleSize)
	{
		// move the metrics back by one
		std::copy(m_metrics + 1, m_metrics + _countof(m_metrics), m_metrics);

		// reset the first metric
		m_metrics[g_netOverlaySampleCount] = NetPacketMetrics();

		// update the timer
		m_lastUpdatePerSample = time;
	}

	if ((time - m_lastUpdatePerSec) > 1000)
	{
		// set 'last' values
		m_lastInBytes = m_inBytes;
		m_lastInPackets = m_inPackets;

		m_lastOutBytes = m_outBytes;
		m_lastOutPackets = m_outPackets;

		// reset 'current' values
		m_inBytes = 0;
		m_inPackets = 0;

		m_outBytes = 0;
		m_outPackets = 0;

		// update the timer
		m_lastUpdatePerSec = time;
	}
}

void NetOverlayMetricSink::DrawGraph()
{
	// calculate maximum height for this data subset
	float maxHeight = 0;

	for (int i = 0; i < _countof(m_metrics); i++)
	{
		auto metric = m_metrics[i];
		auto totalSize = metric.GetTotalSize();

		if (totalSize > maxHeight)
		{
			maxHeight = totalSize;
		}
	}

	// calculate per-sample size
	int perSampleSize = (g_netOverlayWidth / g_netOverlaySampleCount);

	for (int i = 0; i < _countof(m_metrics) - 1; i++) // the last entry is transient, so ignore that
	{
		auto metric = m_metrics[i];

		// base X/Y for this metric
		int x = GetOverlayLeft() + (perSampleSize * i);
		int y = GetOverlayTop() + (g_netOverlayHeight - 100);

		for (int j = 0; j < NET_PACKET_SUB_MAX; j++)
		{
			// get Y for this submetric
			float y1 = y - ((metric.GetElementSize((NetPacketSubComponent)j) / maxHeight) * (g_netOverlayHeight - 100));
			float y2 = y;

			// set a rectangle
			CRect rect(x, y1, x + perSampleSize, y2);
			CRGBA color = GetColorIndex(j);

			TheFonts->DrawRectangle(rect, color);

			// the next one starts where this one left off
			y = y1;
		}
	}
}

CRGBA NetOverlayMetricSink::GetColorIndex(int index)
{
	static CRGBA colorTable[] = {
		CRGBA(0x00, 0x00, 0xAA),
		CRGBA(0x00, 0xAA, 0x00),
		CRGBA(0x00, 0xAA, 0xAA),
		CRGBA(0xAA, 0x00, 0x00),
		CRGBA(0xAA, 0x00, 0xAA),
		CRGBA(0xAA, 0x55, 0x00),
		CRGBA(0x55, 0x55, 0xFF),
		CRGBA(0x55, 0xFF, 0x55),
		CRGBA(0x55, 0xFF, 0xFF),
		CRGBA(0xFF, 0x55, 0x55),
		CRGBA(0xFF, 0x55, 0xFF),
		CRGBA(0xFF, 0xFF, 0x55)
	};

	return colorTable[index % _countof(colorTable)];
}

void NetOverlayMetricSink::DrawBaseMetrics()
{
	// positioning
	int x = GetOverlayLeft();
	int y = GetOverlayTop() + (g_netOverlayHeight - 100) + 10;

	CRGBA color(255, 255, 255);
	CRect rect(x, y, x + (g_netOverlayWidth / 2), y + 100);

	// collecting
	int ping = m_ping;
	int inPackets = m_lastInPackets;
	int outPackets = m_lastOutPackets;

	// drawing
	TheFonts->DrawText(va(L"ping: %dms\nin: %d/s\nout: %d/s", ping, inPackets, outPackets), rect, color, 22.0f, 1.0f, "Lucida Console");

	//
	// second column
	//

	// positioning
	rect.SetRect(rect.fX2, rect.fY1, rect.fX2 + (g_netOverlayWidth / 2), rect.fY2);

	// collecting
	int inBytes = m_lastInBytes;
	int outBytes = m_lastOutBytes;

	// drawing
	TheFonts->DrawText(va(L"\nin: %d b/s\nout: %d b/s", inBytes, outBytes), rect, color, 22.0f, 1.0f, "Lucida Console");
}

static InitFunction initFunction([] ()
{
	// register an event to create a network metric sink
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		fwRefContainer<INetMetricSink> sink = new NetOverlayMetricSink();
		netLibrary->SetMetricSink(sink);
	});
});