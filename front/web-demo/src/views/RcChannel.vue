<template>
  <v-container>
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
        <v-card>
          <v-card-text>
            <v-container fluid grid-list-lg>
              <v-layout row wrap>
                <v-flex xs9>
                  <v-slider v-model="roll" :min="1000" :max="2000" label="Roll"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="roll" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="pitch" :min="1000" :max="2000" label="Pitch"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="pitch" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="throttle" :min="885" :max="2000" label="Throttl"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="throttle" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="yaw" :min="1000" :max="2000" label="Yaw"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="yaw" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <br>
                <v-flex xs9>
                  <v-slider v-model="channel5" :min="1000" :max="2000" label="Channel5"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel5" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel6" :min="1000" :max="2000" label="Channel6"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel6" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel7" :min="1000" :max="2000" label="Channel7"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel7" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel8" :min="1000" :max="2000" label="Channel8"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel8" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel9" :min="1000" :max="2000" label="Channel9"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel9" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel10" :min="1000" :max="2000" label="Channel10"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel10" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel11" :min="1000" :max="2000" label="Channel11"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel11" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel12" :min="1000" :max="2000" label="Channel12"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel12" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel13" :min="1000" :max="2000" label="Channel13"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel13" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel14" :min="1000" :max="2000" label="Channel14"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel14" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel15" :min="1000" :max="2000" label="Channel15"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel15" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel16" :min="1000" :max="2000" label="Channel16"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel16" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel17" :min="1000" :max="2000" label="Channel17"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel17" class="mt-0" type="number"></v-text-field>
                </v-flex>
                <v-flex xs9>
                  <v-slider v-model="channel18" :min="1000" :max="2000" label="Channel18"></v-slider>
                </v-flex>
                <v-flex xs3>
                  <v-text-field v-model="channel18" class="mt-0" type="number"></v-text-field>
                </v-flex>
              </v-layout>
            </v-container>
          </v-card-text>
          <v-btn class="ma-2" color="primary" dark @click="set_rc_channel">
            Apply
          </v-btn>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
export default {
  data() {
    return { roll: 1500, pitch: 1500, throttle: 885, yaw: 1500, 
      channel5: 1200, 
      channel6: 1200, 
      channel7: 1200, 
      channel8: 1200, 
      channel9: 1200, 
      channel10: 1200, 
      channel11: 1200, 
      channel12: 1200, 
      channel13: 1200, 
      channel14: 1200, 
      channel15: 1200, 
      channel16: 1200, 
      channel17: 1200, 
      channel18: 1200  };
  },
  mounted() {
    this.$ajax
      .get("/api/v1/rc/raw")
      .then(data => {
        this.roll      = data.data.ch_0;
        this.pitch     = data.data.ch_1;
        this.throttle  = data.data.ch_2;
        this.yaw       = data.data.ch_3;
        this.channel5  = data.data.ch_4;
        this.channel6  = data.data.ch_5;
        this.channel7  = data.data.ch_6;
        this.channel8  = data.data.ch_7;
        this.channel9  = data.data.ch_8;
        this.channel10 = data.data.ch_9;
        this.channel11 = data.data.ch_10;
        this.channel12 = data.data.ch_11;
        this.channel13 = data.data.ch_12;
        this.channel14 = data.data.ch_13;
        this.channel15 = data.data.ch_14;
        this.channel16 = data.data.ch_15;
        this.channel17 = data.data.ch_16;
        this.channel18 = data.data.ch_17
      })
      .catch(error => {
        console.log(error);
      });
  },
  methods: {
    set_rc_channel: function() {
      this.$ajax
        .post("/api/v1/rc/post", {
          ch_0: this.roll,
          ch_1: this.pitch,
          ch_2: this.throttle,
          ch_3: this.yaw,
          ch_4: this.channel5,
          ch_5: this.channel6,
          ch_6: this.channel7,
          ch_7: this.channel8,
          ch_8: this.channel9,
          ch_9: this.channel10,
          ch_10: this.channel11,
          ch_11: this.channel12,
          ch_12: this.channel13,
          ch_13: this.channel14,
          ch_14: this.channel15,
          ch_15: this.channel16,
          ch_16: this.channel17,
          ch_17: this.channel18
        })
        .then(data => {
          console.log(data);
        })
        .catch(error => {
          console.log(error);
        });
    },
  }
};
</script>
