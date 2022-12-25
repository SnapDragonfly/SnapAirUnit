<template>
    <v-container
    class="px-0"
    fluid
  >
    <v-row>
      <v-col
        cols="12"
        sm="6"
        md="3"
      >
        <v-text-field v-model="ap_ssid"
          label="WiFi AP SSID"
          placeholder="SnapAirUnitTest"
          ></v-text-field>
      </v-col>

      <v-col
        cols="12"
        sm="6"
        md="3"
      >
        <v-text-field v-model="ap_pass"
          label="WiFi AP Password"
          placeholder="12345678Test"
          ></v-text-field>
      </v-col>
    </v-row>

    <v-btn class="ma-2" color="primary" dark @click="set_ap">
        Save AP Configuration
    </v-btn>

    <v-row>
      <v-col
        cols="12"
        sm="6"
        md="3"
      >
        <v-text-field v-model="sta_ssid"
          label="WiFi Station SSID"
          placeholder="AutoLabTest"
          ></v-text-field>
      </v-col>

      <v-col
        cols="12"
        sm="6"
        md="3"
      >
        <v-text-field v-model="sta_pass"
          label="WiFi Station Password"
          placeholder="68686868Test"
          ></v-text-field>
      </v-col>
      <v-btn class="ma-2" color="primary" dark @click="set_sta">
        Save Station Configuration
      </v-btn>
    </v-row>
    <v-radio-group v-model="wireless">
      <v-radio :label="`WiFi Ap`" :value=0></v-radio>
      <v-radio :label="`WiFi Station`" :value=1></v-radio>
      <v-radio :label="`Bluetooth SPP UART`" :value=2></v-radio>
    </v-radio-group>

    <v-btn class="ma-2" color="primary" dark @click="set_wireless_mode">
      Apply
    </v-btn>
  </v-container>
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
