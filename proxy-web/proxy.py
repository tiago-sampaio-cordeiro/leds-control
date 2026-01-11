#!/usr/bin/env python3
"""
Proxy CoAP <-> HTTP para controle de LEDs CC2650 com suporte a blink
"""

import asyncio
from aiohttp import web
from aiocoap import Context, Message, POST

MOTES = {
    "mote1": "coap://[fd00::212:4b00:d77:8403]",
    "mote2": "coap://[fd00::212:4b00:d6a:be83]",
}

# handler leds
async def handle_led_control(request):
    try:
        data = await request.json()
        mote = data.get("mote")
        led = data.get("led")
        action = data.get("action")

        if not mote or not led or not action:
            return web.json_response({"error": "Campos obrigatórios: mote, led, action"}, status=400)

        if mote not in MOTES:
            return web.json_response({"error": "Mote inválido"}, status=404)

        if led not in ("red", "green"):
            return web.json_response({"error": "LED inválido"}, status=400)

        if action not in ("on", "off", "blink"):
            return web.json_response({"error": "Ação inválida"}, status=400)

        # Payload para o mote
        if action == "blink":
            payload = f"{led}-blink".encode()  # comando para o mote piscar
        else:
            payload = f"{led}-{action}".encode()  # red-on, green-off, etc.

        uri = f"{MOTES[mote]}/actuators/led"

        context = await Context.create_client_context()
        req = Message(code=POST, uri=uri, payload=payload)

        res = await context.request(req).response

        return web.json_response({
            "status": "ok",
            "mote": mote,
            "led": led,
            "action": action,
            "coap_code": str(res.code)
        })

    except Exception as e:
        return web.json_response({"error": str(e)}, status=500)

async def index(request):
    return web.FileResponse("index.html")

def main():
    app = web.Application()
    app.router.add_get("/", index)
    app.router.add_post("/api/led/control", handle_led_control)

    print("="*60)
    print("🚀 Proxy CoAP ↔ HTTP ONLINE")
    print("🌐 http://localhost:8080")
    print("📡 POST /api/led/control")
    print("⚠️  IMPORTANTE: Endereços IPv6 corretos no dicionário MOTES")
    print("="*60)

    web.run_app(app, host="0.0.0.0", port=8080)

if __name__ == "__main__":
    main()
