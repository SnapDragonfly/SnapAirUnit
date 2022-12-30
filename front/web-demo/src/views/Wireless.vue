<template>
  <v-form>
    <v-container
      class="px-0"
      fluid
    >
      <v-row>
        <v-col
          cols="12"
          sm="6"
        >
          <v-text-field v-model="ap_ssid"
            :rules="[
                () => !!ap_ssid || 'This field is required',
                () => !!ap_ssid && ap_ssid.length <= 15 || 'SSID must be less than 15 characters',
                () => !!ap_ssid && ap_ssid.length >= 4 || 'SSID must be more than 4 characters',
                ap_ssidCheck
              ]"
            :onkeyup="ap_ssid = ap_ssid.replace(/\s+/g,'')"
            label="WiFi AP SSID"
            filled
            shaped
            placeholder="SnapAirUnitTest"
            ></v-text-field>
        </v-col>

        <v-col
          cols="12"
          sm="6"
        >
          <v-text-field v-model.trim="ap_pass"
            :append-icon="ap_show ? 'iconfont icon-eye' : 'iconfont icon-eye-close'"
            :type="ap_show ? 'text' : 'password'"
            :rules="[
                () => !!ap_pass || 'This field is required',
                () => !!ap_pass && ap_pass.length <= 15 || 'Password must be less than 15 characters',
                () => !!ap_pass && ap_pass.length >= 8 || 'Password must be more than 8 characters',
                ap_passCheck
              ]"
            :onkeyup="ap_pass = ap_pass.replace(/\s+/g,'')"
            label="WiFi AP Password"
            filled
            shaped
            placeholder="12345678Test"
            counter="15"
            hint="At least 8 characters"
            @click:append="ap_show = !ap_show"
            ></v-text-field>
        </v-col>

        <v-btn class="ma-2" color="primary" dark @click="set_ap">
          Save AP Configuration
        </v-btn>
      </v-row>

      <v-row>
        <v-col
          cols="12"
          sm="6"
        >
          <v-text-field v-model="sta_ssid"
            :rules="[
                () => !!sta_ssid || 'This field is required',
                () => !!sta_ssid && sta_ssid.length <= 15 || 'SSID must be less than 15 characters',
                () => !!sta_ssid && sta_ssid.length >= 4 || 'SSID must be more than 4 characters',
                sta_ssidCheck
              ]"
            :onkeyup="sta_ssid = sta_ssid.replace(/\s+/g,'')"
            label="WiFi Station SSID"
            filled
            shaped
            placeholder="AutoLabTest"
            ></v-text-field>
        </v-col>

        <v-col
          cols="12"
          sm="6"
        >
          <v-text-field v-model.trim="sta_pass"
            :append-icon="sta_show ? 'iconfont icon-eye' : 'iconfont icon-eye-close'"
            :type="sta_show ? 'text' : 'password'"
            :rules="[
                () => !!sta_pass || 'This field is required',
                () => !!sta_pass && sta_pass.length <= 15 || 'Password must be less than 15 characters',
                () => !!sta_pass && sta_pass.length >= 8 || 'Password must be more than 8 characters',
                sta_passCheck
              ]"
            :onkeyup="sta_pass = sta_pass.replace(/\s+/g,'')"
            label="WiFi Station Password"
            filled
            shaped
            placeholder="68686868Test"
            counter="15"
            hint="At least 8 characters"
            @click:append="sta_show = !sta_show"
            ></v-text-field>
        </v-col>
        <v-btn class="ma-2" color="primary" dark @click="set_sta">
          Save Station Configuration
        </v-btn>
      </v-row>

      <v-row>
        <v-radio-group v-model="wireless">
          <v-radio :label="`WiFi Ap`" :value=0></v-radio>
          <v-radio :label="`WiFi Station`" :value=1></v-radio>
          <v-radio :label="`Bluetooth SPP UART`" :value=2></v-radio>
        </v-radio-group>

        <v-btn class="ma-2" color="primary" dark @click="set_wireless_mode">
          Apply Wireless Mode
        </v-btn>
      </v-row>

    </v-container>
  </v-form>
</template>

<script>
export default {
  data() {
    return {
      ap_ssid: null,
      ap_pass: null,
      sta_ssid: null,
      sta_pass: null,
      wireless: null,
      ap_show: false,
      sta_show: false,
    };
  },
  methods: {
    set_wireless_mode: function() {
      this.$ajax
      .post("/api/v1/wireless/post", {
          wireless: this.wireless
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    },
    set_ap: function() {
      this.$ajax
      .post("/api/v1/ap/post", {
          ssid: this.ap_ssid,
          pass: this.ap_pass
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    },
    set_sta: function() {
      this.$ajax
      .post("/api/v1/sta/post", {
          ssid: this.sta_ssid,
          pass: this.sta_pass
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    },
    get_wireless_mode: function() {
      this.$ajax
      .get("/api/v1/wireless/raw")
      .then(data => {
        this.wireless = data.data.wireless
      })
      .then(data => {
        console.log(data);
      })
      .catch(error => {
        console.log(error);
      });
    },
    get_ap:function(){
      this.$ajax
      .get("/api/v1/ap/raw")
      .then(data => {
        this.ap_ssid = data.data.ssid;
        this.ap_pass = data.data.pass;
      })
      .then(data => {
        console.log(data);
      })
      .catch(error => {
        console.log(error);
      });
    },
    get_sta:function(){
      this.$ajax
      .get("/api/v1/sta/raw")
      .then(data => {
        this.sta_ssid = data.data.ssid;
        this.sta_pass = data.data.pass;
      })
      .then(data => {
        console.log(data);
      })
      .catch(error => {
        console.log(error);
      });
    }
  },
  mounted() {
    this.get_wireless_mode();
    this.get_ap();
    this.get_sta();
  }
};
</script>
