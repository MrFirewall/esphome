import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_OTA, CONF_WIFI
from esphome.components import sensor
from esphome.components.ota import OTAComponent
from esphome.components.wifi import WiFiComponent

DEPENDENCIES = ['logger', 'proxy_base', 'ota', 'wifi']
AUTO_LOAD = ['proxy_base']

CONF_ESPNOW_CHANNEL = "espnow_channel"
CONF_RECEIVER_MAC_ADDRESS = "receiver_mac_address"
CONF_SENSORS = "sensors"

CONF_SENSOR_SENSOR = "sensor"
CONF_SENSOR_PROXY_ID = "proxy_id"

ns = cg.esphome_ns.namespace('proxy_transmitter')
ProxyTransmitterComponent = ns.class_(
    'ProxyTransmitterComponent', cg.Component)
PeerReceiver = ns.class_(
    'PeerReceiver')


def validate_proxy_id(value):
    value = cv.string_strict(value)
    value = cv.Length(max=20)(value)
    return value


SENSOR_SCHEMA = cv.Schema({
    cv.Required(CONF_SENSOR_SENSOR): cv.use_id(sensor.Sensor),
    cv.Required(CONF_SENSOR_PROXY_ID): validate_proxy_id,
})

RECEIVER_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PeerReceiver),
})


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ProxyTransmitterComponent),
    cv.Required(CONF_ESPNOW_CHANNEL): cv.int_,
    cv.Required(CONF_RECEIVER_MAC_ADDRESS): cv.mac_address,
    cv.Required(CONF_SENSORS): cv.ensure_list(SENSOR_SCHEMA)
}).extend({cv.GenerateID(CONF_OTA): cv.use_id(OTAComponent)
           }).extend({cv.GenerateID(CONF_WIFI): cv.use_id(WiFiComponent)})


def to_code(config):
    ota = yield cg.get_variable(config[CONF_OTA])
    wifi = yield cg.get_variable(config[CONF_WIFI])

    transmitter_var = cg.new_Pvariable(config[CONF_ID])
    # Consigure transmitter component
    cg.add(transmitter_var.set_wifi(wifi))
    yield cg.register_component(transmitter_var, config)

    peer_receiver_id = cv.declare_id(PeerReceiver)("peer_receiver")
    peer_receiver_var = cg.new_Pvariable(peer_receiver_id)
    # Configure peer receiver component

    cg.add(peer_receiver_var.set_espnow_channel(
        config[CONF_ESPNOW_CHANNEL]))
    cg.add(peer_receiver_var.set_mac_address(
        config[CONF_RECEIVER_MAC_ADDRESS].as_hex))

    cg.add(peer_receiver_var.set_name(
        "receiver"))
    # Add peer receiver to transmitter component
    cg.add(transmitter_var.set_peer_receiver(peer_receiver_var))

    cg.add(peer_receiver_var.set_ota(ota))

    #cg.add(peer_receiver_var.set_wifi(wifi))

    for sensor_config in config.get(CONF_SENSORS, []):
        sensor = yield cg.get_variable(sensor_config[CONF_SENSOR_SENSOR])
        cg.add(peer_receiver_var.add_sensor(
            sensor,
            sensor_config[CONF_SENSOR_PROXY_ID]))
