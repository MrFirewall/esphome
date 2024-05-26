import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_OTA, CONF_WIFI
from esphome.components import sensor
from esphome.components.wifi import WiFiComponent
from esphome.components.ota import OTAComponent

DEPENDENCIES = ['logger', 'ota', 'wifi']
AUTO_LOAD = ['proxy_base', 'sensor', 'switch']

CONF_ESPNOW_CHANNEL = "espnow_channel"
CONF_TRANSMITTERS = "transmitters"

CONF_TRANSMITTER_MAC_ADDRESS = "mac_address"
CONF_TRANSMITTER_NAME = "name"
CONF_TRANSMITTER_PROXIED_SENSORS = "proxied_sensors"

CONF_PROXIED_SENSOR_PROXY_ID = "proxy_id"
CONF_PROXIED_SENSOR_NAME = "name"

ns = cg.esphome_ns.namespace('proxy_receiver')
ProxyReceiverComponent = ns.class_('ProxyReceiverComponent', cg.Component)
ProxiedSensorComponent = ns.class_(
    'ProxiedSensorComponent', cg.Component, sensor.Sensor)
PeerTransmitter = ns.class_(
    'PeerTransmitter')


def validate_proxy_id(value):
    value = cv.string_strict(value)
    value = cv.Length(max=20)(value)
    return value


PROXIED_SENSOR_SCHEMA = cv.All(
    sensor.sensor_schema(
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(ProxiedSensorComponent),
            cv.Required(CONF_PROXIED_SENSOR_PROXY_ID): validate_proxy_id,
        }
    ))

TRANSMITTER_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PeerTransmitter),
    cv.Required(CONF_TRANSMITTER_MAC_ADDRESS): cv.mac_address,
    cv.Required(CONF_TRANSMITTER_NAME): cv.string_strict,
    cv.Required(CONF_TRANSMITTER_PROXIED_SENSORS): cv.ensure_list(PROXIED_SENSOR_SCHEMA)
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ProxyReceiverComponent),
    cv.Required(CONF_ESPNOW_CHANNEL): cv.int_,
    cv.Required(CONF_TRANSMITTERS): cv.ensure_list(TRANSMITTER_SCHEMA),
}).extend({cv.GenerateID(CONF_OTA): cv.use_id(OTAComponent)
}).extend({cv.GenerateID(CONF_WIFI): cv.use_id(WiFiComponent)})



def to_code(config):
    
 
    # ota = yield cg.get_variable(config[CONF_OTA])
    # cg.add(peer_transmitter_var.set_ota(ota))

    # wifi = yield cg.get_variable(config[CONF_WIFI])
    # cg.add(peer_transmitter_var.set_wifi(wifi))   
    # Create receiver component
    receiver_var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(receiver_var, config)
    # Configure receiver component


    for transmitterConf in config.get(CONF_TRANSMITTERS, []):
        # Create the peer transmitter
        peer_transmitter_var = cg.new_Pvariable(transmitterConf[CONF_ID])
        # Configure peer receiver component
        cg.add(peer_transmitter_var.set_espnow_channel(
            config[CONF_ESPNOW_CHANNEL]))
        cg.add(peer_transmitter_var.set_mac_address(
            transmitterConf[CONF_TRANSMITTER_MAC_ADDRESS].as_hex))
        cg.add(peer_transmitter_var.set_name(
            transmitterConf[CONF_TRANSMITTER_NAME]))
 

        # Add peer transmitter to receiver component
        cg.add(receiver_var.add_peer_transmitter(peer_transmitter_var))

        for proxiedSensorConf in transmitterConf.get(CONF_TRANSMITTER_PROXIED_SENSORS, []):
            # Crate proxied sensor
            proxied_sesnor_var = cg.new_Pvariable(proxiedSensorConf[CONF_ID])
            # Configure proxied sensor
            cg.add(proxied_sesnor_var.set_proxied_sensor_id(
                proxiedSensorConf[CONF_PROXIED_SENSOR_PROXY_ID]))
            # Add proxied sensor to peer transmitter
            cg.add(peer_transmitter_var.add_proxied_sensor(proxied_sesnor_var))
            # Register proxied sensor
            yield sensor.register_sensor(proxied_sesnor_var, proxiedSensorConf)
            yield cg.register_component(proxied_sesnor_var, proxiedSensorConf)
