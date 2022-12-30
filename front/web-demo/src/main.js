import Vue from 'vue'
import './plugins/vuetify'
import App from './App.vue'
import router from './router'
import axios from 'axios'
import '@/assets/iconfont/iconfont.css'

Vue.config.productionTip = false

Vue.prototype.$ajax = axios

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
