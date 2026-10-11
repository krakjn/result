(function () {
  var root = document.documentElement;

  var toggle = document.querySelector(".theme-toggle");
  if (toggle) {
    toggle.addEventListener("click", function () {
      var next = root.dataset.theme === "dark" ? "light" : "dark";
      root.dataset.theme = next;
      localStorage.setItem("theme", next);
    });
  }

  if (window.hljs) {
    document.querySelectorAll("pre code").forEach(function (block) {
      if (block.classList.contains("language-stdout") || block.classList.contains("language-stderr")) {
        return;
      }
      window.hljs.highlightElement(block);
    });
  }

  document.querySelectorAll(".frame pre").forEach(function (pre) {
    var button = document.createElement("button");
    button.type = "button";
    button.className = "copy";
    button.textContent = "Copy";
    button.addEventListener("click", function () {
      navigator.clipboard.writeText(pre.querySelector("code").innerText).then(function () {
        button.textContent = "Copied";
        button.classList.add("done");
        setTimeout(function () {
          button.textContent = "Copy";
          button.classList.remove("done");
        }, 1400);
      });
    });
    pre.appendChild(button);
  });

  var bar = document.querySelector(".progress");
  function progress() {
    var max = document.documentElement.scrollHeight - window.innerHeight;
    bar.style.transform = "scaleX(" + (max > 0 ? window.scrollY / max : 0) + ")";
  }
  window.addEventListener("scroll", progress, { passive: true });
  progress();

  var links = {};
  document.querySelectorAll(".toc a[href^='#']").forEach(function (a) {
    links[decodeURIComponent(a.getAttribute("href").slice(1))] = a;
  });
  var headings = document.querySelectorAll(".frame h2[id], .frame h3[id]");
  if (headings.length && "IntersectionObserver" in window) {
    var observer = new IntersectionObserver(
      function (entries) {
        entries.forEach(function (entry) {
          if (!entry.isIntersecting) return;
          Object.keys(links).forEach(function (id) {
            links[id].classList.toggle("active", id === entry.target.id);
          });
        });
      },
      { rootMargin: "0px 0px -70% 0px" }
    );
    headings.forEach(function (h) {
      observer.observe(h);
    });
  }
})();
