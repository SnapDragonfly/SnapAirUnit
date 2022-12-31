<template>
  <v-container>
    <v-layout text-xs-center wrap>
      <v-flex xs12 sm6 offset-sm3>
        <v-card>
          <v-img :src="require('../assets/logo.png')" contain height="200"></v-img>
          <v-card-title primary-title>
            <div class="ma-auto">
              <span class="grey--text">APP version: {{app_version}}</span>
              <br>
              <span class="grey--text">IDF version: {{sdk_version}}</span>
            </div>
            <hr>
            <div class="ma-auto">
              <span class="grey--text">ESP model: {{model}}, cores: {{cores}}, revision: {{revision}}</span>
              <br>
              <span class="grey--text">stack: {{simplified_version}}</span>
            </div>
          </v-card-title>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
export default {
  data() {
    return {
      app_version: null,
      sdk_version: null,
      model: null,
      cores: null,
      revision: null,
      simplified: null,
      simplified_version: null,
    };
  },
  mounted() {
    this.$ajax
      .get("/api/v1/system/info")
      .then(data => {
        this.app_version = data.data.app_version;
        this.sdk_version = data.data.sdk_version;
        this.model = data.data.model;
        this.cores = data.data.cores;
        this.revision = data.data.revision;
        this.simplified = data.data.simplified;
        this.simplified_version = this.simplified == 1 ? "simplified" : "full";
      })
      .catch(error => {
        console.log(error);
      });
  }
};
</script>
