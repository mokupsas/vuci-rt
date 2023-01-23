<template>
  <div class="test">

      <vuci-form uci-config="mosquitto">

          <vuci-named-section title="MQTT Broker" name="mqtt" v-slot="{ s }" style="margin-bottom: 20px">
              <vuci-form-item-switch :uci-section="s" label="Enable" name="enabled"/>
              <vuci-form-item-input :uci-section="s" label="Local port" name="local_port"/>
              <vuci-form-item-switch :uci-section="s" label="Enable remote access" name="enable_ra"/>
          </vuci-named-section>

          <vuci-named-section title="Broker Settings" name="mqtt" v-slot="{ s }" style="margin-bottom: 20px">
              <a-tabs>
                  <a-tab-pane key="securrity" tab="Security">
                      <vuci-form-item-switch :uci-section="s" label="Use TLS/SSL" name="use_tls_ssl"/>
                      <vuci-form-item-select :uci-section="s" label="TLS Type" name="tls_type" :options="tls_types" depend="use_tls_ssl == 1"/>
                      <vuci-form-item-upload :uci-section="s" label="CA file" name="ca_file" depend="use_tls_ssl == 1 && tls_type == 'cert'"/>
                      <vuci-form-item-upload :uci-section="s" label="Cert file" name="cert_file" depend="use_tls_ssl == 1 && tls_type == 'cert'"/>
                      <vuci-form-item-upload :uci-section="s" label="Key file" name="key_file" depend="use_tls_ssl == 1 && tls_type == 'cert'"/>
                      <vuci-form-item-select :uci-section="s" label="TLS version" name="tls_version" :options="tls_versions" depend="use_tls_ssl == 1 && tls_type == 'cert'"/>
                      <vuci-form-item-input :uci-section="s" label="Pre-Shared-Key" name="psk" depend="use_tls_ssl == 1 && tls_type == 'psk'"/>
                      <vuci-form-item-input :uci-section="s" label="Identity" name="identity" depend="use_tls_ssl == 1 && tls_type == 'psk'"/>
                  </a-tab-pane>
                  <a-tab-pane key="miscellaneous" tab="Miscellaneous">
                    <vuci-form-item-upload :uci-section="s" label="ACL file" name="acl_file_path"/>
                    <vuci-form-item-upload :uci-section="s" label="Password file" name="password_file"/>
                    <vuci-form-item-switch :uci-section="s" label="Persistence" name="persistence"/>
                    <vuci-form-item-switch :uci-section="s" label="Allow Anonymous" name="anonymous_access"/>
                  </a-tab-pane>
              </a-tabs>
          </vuci-named-section>

          <!--
          <a-collapse>
              <a-collapse-panel key="1" header="MQTT BROKER">
                  <p>asda</p>
              </a-collapse-panel>
          </a-collapse>
          <a-collapse>
              <a-collapse-panel key="1" header="MQTT BROKER">
                  <p>asda</p>
              </a-collapse-panel>
          </a-collapse>
          -->
          <!--
          <a-tabs>
              <a-tab-pane key="system" tab="MQTT Broker">

              </a-tab-pane>
              <a-tab-pane key="logging" tab="Settings">

              </a-tab-pane>
          </a-tabs>
          -->
      </vuci-form>

  </div>
</template>

<script>
export default {
data () {
  return {
    time: '',
    tls_types: [
      ['cert', 'Certificate based'],
      ['psk', 'Pre-Shared-Key base']
    ],
    tls_versions: [
      ['all', 'Support all'],
      ['tlsv1', 'TLSV1'],
      ['tlsv1.1', 'TLSV1.1'],
      ['tlsv1.2', 'TLSV1.2']
    ]
  }
},
methods: {
  get_time () {
    this.$rpc.call('example', 'get_time', {}).then(r => {
      this.time = r.time
    })
  }
}
}
</script>

<style></style>
